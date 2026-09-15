# Datenschema

## Hierarchie

```
Space          öffentlich | auf Anfrage | nur Einladung
  Participant  Anzeigename im Space, optional an Account gebunden
  Topic        eine Diskussion
    Room       Opener oder Abzweig
      Message
      Section  Agent-Segmentierung
      Question Agent-Extraktion mit Status
```

---

## 1. Accounts und Zugang

### user
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| email | text unique | |
| password_hash | text NULL | NULL bei reinem OAuth |
| display_name | text | |
| avatar_url | text NULL | |
| totp_secret | text NULL | 2FA-Modul |
| created_at | timestamptz | |

### oauth_identity
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| user_id | uuid FK user | |
| provider | enum | discord, google, 42 |
| provider_user_id | text | |
| created_at | timestamptz | |

unique(provider, provider_user_id)

---

## 2. Spaces

### space
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| slug | text unique | |
| name | text | |
| description | text | |
| visibility | enum | public, on_request, invite_only |
| origin | enum | native, imported |
| created_by | uuid FK user | |
| created_at | timestamptz | |

**Regel:** `origin = imported` erzwingt `visibility = invite_only`. Als Check-Constraint, nicht nur in der Anwendungslogik.

### space_member
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| space_id | uuid FK space | |
| user_id | uuid FK user | |
| role | enum | owner, admin, moderator, member |
| joined_at | timestamptz | |

unique(space_id, user_id)

### join_request
Nur für `visibility = on_request`.

| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| space_id | uuid FK space | |
| user_id | uuid FK user | |
| status | enum | pending, accepted, rejected |
| decided_by | uuid FK user NULL | |
| decided_at | timestamptz NULL | |
| created_at | timestamptz | |

unique(space_id, user_id) where status = pending

### invite
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| space_id | uuid FK space | |
| token | text unique | zufällig, min. 32 Byte |
| created_by | uuid FK user | |
| max_uses | int NULL | NULL = unbegrenzt |
| use_count | int default 0 | |
| expires_at | timestamptz NULL | |
| revoked_at | timestamptz NULL | |

---

## 3. Teilnehmer — der zentrale Kniff

### participant
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| space_id | uuid FK space | am Space, nicht am Room |
| display_name | text | "astehl (Barbara)" oder "Teilnehmer 3" |
| source_handle | text NULL | "astehl", NULL bei Anonymisierung |
| source_external_id | text NULL | Discord-User-ID o.ä. |
| user_id | uuid FK user NULL | NULL = Karteileiche |
| claimed_at | timestamptz NULL | |
| claim_approved_by | uuid FK user NULL | Importeur bestätigt |

unique(space_id, user_id) where user_id is not null
→ ein Account kann pro Space nur einen Teilnehmer beanspruchen

**Regel:** `claimed_at` einmal gesetzt ist endgültig. Kein Zurücksetzen, sonst ist es ein Identitätsloch.

**Regel:** Jede Nachricht zeigt auf `participant`, niemals direkt auf `user`. Auch die von echten Nutzern. Ein Nutzer, der einem Space beitritt, bekommt automatisch einen Participant mit gesetzter `user_id`.

---

## 4. Diskussionen

### topic
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| space_id | uuid FK space | |
| title | text | |
| created_by_participant_id | uuid FK participant | |
| created_at | timestamptz | |
| last_activity_at | timestamptz | |
| room_count | int | denormalisiert für die Liste |

### room
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| topic_id | uuid FK topic | |
| parent_room_id | uuid FK room NULL | NULL = Opener |
| anchor_message_id | uuid FK message NULL | NULL = Opener |
| title | text | |
| created_by_participant_id | uuid FK participant | |
| origin | enum | manual, agent |
| created_at | timestamptz | |
| message_count | int | |
| last_message_at | timestamptz NULL | |
| summary_dirty | bool default true | |

unique(topic_id) where parent_room_id is null
→ genau ein Opener pro Topic

**Regel:** `anchor_message_id` muss zu `parent_room_id` gehören. Check beim Anlegen.

### message
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| room_id | uuid FK room | |
| author_participant_id | uuid FK participant | |
| body | text | |
| reply_to_message_id | uuid FK message NULL | aus der Quelle übernommen |
| source_external_id | text NULL | für Dedupe bei Re-Import |
| created_at | timestamptz | Originalzeit, nicht Importzeit |
| edited_at | timestamptz NULL | |
| deleted_at | timestamptz NULL | soft delete |

unique(room_id, source_external_id) where source_external_id is not null

Index: `(room_id, created_at)` — trägt die Pagination in beide Richtungen.

---

## 5. Agent-Layer

### section
Segmentierung nach inhaltlicher Abschweifung.

| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| room_id | uuid FK room | |
| idx | int | Reihenfolge im Raum |
| from_message_id | uuid FK message | |
| to_message_id | uuid FK message | |
| summary | text | |
| model_version | text | für Reproduzierbarkeit |
| created_at | timestamptz | |

unique(room_id, idx)

**Regel:** Mindestlänge pro Section (z.B. 5 Nachrichten), sonst zerfällt ein lebhafter Chat in Dutzende Mini-Abschnitte.

### question
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| room_id | uuid FK room | |
| asked_by_participant_id | uuid FK participant | |
| message_id | uuid FK message | die Frage selbst |
| text | text | normalisierte Fassung |
| status | enum | open, answered, dismissed |
| answer_message_id | uuid FK message NULL | |
| status_source | enum | agent, human |
| status_changed_by | uuid FK user NULL | |
| status_changed_at | timestamptz NULL | |
| detected_at | timestamptz | |

**Regel:** `status_source = agent` ist immer nur ein Vorschlag. Jede menschliche Korrektur setzt `status_source = human` und wird nie wieder vom Agenten überschrieben.

### summary_job
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| room_id | uuid FK room | |
| status | enum | queued, running, done, failed |
| attempts | int default 0 | |
| last_error | text NULL | |
| created_at | timestamptz | |
| finished_at | timestamptz NULL | |

Ablauf: Schreiben setzt `room.summary_dirty = true`. Worker sammelt dirty Räume, legt Jobs an, verarbeitet sie außerhalb des Request-Pfads.

---

## 6. Import

### import_job
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| space_id | uuid FK space | |
| user_id | uuid FK user | wer importiert hat |
| source | enum | whatsapp_txt, discord_json, telegram_json, raw_text |
| anonymize | bool | Schalter beim Upload |
| status | enum | queued, parsing, done, failed |
| message_count | int NULL | |
| participant_count | int NULL | |
| error | text NULL | |
| created_at | timestamptz | |

**Kanonische Zwischenform** — jeder Parser gibt das aus, nichts dahinter kennt die Quelle:

```json
{
  "source": "whatsapp_txt",
  "external_id": "…",
  "author_name": "astehl (Barbara)",
  "author_external_id": "…",
  "timestamp": "2026-07-20T14:48:00Z",
  "body": "… who deleted the name-row?",
  "reply_to_external_id": null
}
```

Bei `anonymize = true`: `author_name` wird zu "Teilnehmer N", `author_external_id` und `source_handle` bleiben leer.

---

## 7. Benachrichtigungen und Lesestand

### notification
| Feld | Typ | Hinweis |
|---|---|---|
| id | uuid PK | |
| user_id | uuid FK user | |
| kind | enum | mention, answer, branch_created, question_stale, join_request |
| space_id | uuid FK space | |
| room_id | uuid FK room NULL | |
| message_id | uuid FK message NULL | |
| read_at | timestamptz NULL | |
| created_at | timestamptz | |

`question_stale` ist der persönliche Digest: eine Frage von mir ist seit X Stunden offen.

### read_state
| Feld | Typ | Hinweis |
|---|---|---|
| user_id | uuid FK user | |
| room_id | uuid FK room | |
| last_read_message_id | uuid FK message NULL | |
| updated_at | timestamptz | |

PK(user_id, room_id)

---

## 8. Der AI-Layer wird nicht gespeichert

Die Übersichtsansicht ist eine Abfrage, keine Tabelle:

1. Alle `room`s des Topics laden
2. Alle `section`s dieser Räume laden
3. Zusammenführen, sortiert nach Zeitstempel der Ankernachricht bzw. der ersten Nachricht der Section
4. Opener oben festpinnen
5. Offene `question`s als Badge danebenlegen

**Warum:** Eine Tabelle weniger, und die Ansicht kann nie inkonsistent zu den Daten werden.

**Flach halten.** Räume dürfen beliebig tief verzweigen, die Liste bleibt einstufig. Tiefe erscheint nur als Herkunftslabel ("aus: Warum ist der Himmel blau"), nie als Einrückung. Das ist die Lehre aus Google Wave.

---

## 9. Löschkaskade

Space löschen entfernt: Topics, Rooms, Messages, Sections, Questions, Participants, Invites, Import-Jobs, Notifications, Read-States.

Eine einzelne Person entfernen (Anfrage von jemandem ohne Account): `participant.display_name` auf "Entfernt" setzen, `source_handle` und `source_external_id` leeren, ihre Messages soft-deleten, betroffene Räume auf `summary_dirty` setzen, damit die Zusammenfassungen neu geschrieben werden.

Das ist der Punkt, den die meisten vergessen: Die Zusammenfassung enthält den Namen auch dann noch, wenn die Nachricht weg ist.

---

## 10. Modulzuordnung

| Modul | Typ | Pkt | Wo im Schema |
|---|---|---|---|
| Framework Front + Back | Major | 2 | — |
| Real-time WebSockets | Major | 2 | message, room, Präsenz |
| User interaction | Major | 2 | message, participant, friends |
| Standard User Management | Major | 2 | user, oauth_identity |
| Organization system | Major | 2 | space, space_member |
| Advanced permissions | Major | 2 | space_member.role, join_request |
| Modules of choice: Agent-Layer | Major | 2 | section, question, summary_job |
| ORM | Minor | 1 | — |
| Advanced search | Minor | 1 | message, section |
| Notification system | Minor | 1 | notification |
| Custom design system | Minor | 1 | — |
| Analytics dashboard | Minor | 1 | Aggregate über question, room |
| OAuth 2.0 | Minor | 1 | oauth_identity |
| Data export/import | Minor | 1 | import_job |

**Summe: 21 Punkte.** Bonus zählt maximal 5 über die 14 hinaus, also sind 19 die sinnvolle Obergrenze — zwei Module lassen sich streichen oder als Reserve halten, falls im Review etwas nicht validiert wird.
