# Projektstand — Chat mit seitlichen Einstiegspunkten

---

## 1. Das Problem

Chats wachsen nach unten. Wichtiges verschwindet nach oben, man scrollt und sucht. Anpinnen hilft nicht:

- **WhatsApp**: max. 3 Nachrichten, befristet auf 24 Std., 7 oder 30 Tage. Kein dauerhaftes Anpinnen.
- **Discord**: 50 pro Channel, unbefristet. Reicht in gewachsenen Servern nicht — es gibt einen Bot (Pinbot), dessen einziger Zweck das Umgehen dieses Limits ist.

Und Anpinnen ist ohnehin nur ein Lesezeichen:
- Kein Weg zurück — du springst mitten in die Historie und stehst orientierungslos da
- Schreiben geht trotzdem ans Ende, losgelöst vom Bezug
- Ein Pin ist ein Punkt, kein Abschnitt. Wichtig ist aber selten eine Nachricht, sondern ein Verlauf.

---

## 2. Die Lösung

Der Chat wächst **seitlich** statt nach unten.

Aus jeder Nachricht kann ein **Einstiegspunkt** entstehen. Ein Einstiegspunkt ist kein Lesezeichen, sondern ein **eigener Raum** mit eigenem Eingabefeld. Wer dort antwortet, schreibt dorthin — nicht ans Ende des Hauptverlaufs.

Darüber liegt der **AI-Layer**: eine Übersicht, die den Verlauf in Abschnitte zerlegt, zusammenfasst, Fragen erkennt und deren Status anzeigt.

---

## 3. Struktur

```
Space          öffentlich | auf Anfrage | nur Einladung
  Topic        eine Diskussion
    Room       Opener oder Abzweig
      Message
```

**Space** ist der Container — eine Nische, eine Orga, eine private Gruppe.
**Topic** ist eine Diskussion darin.
**Room** ist entweder der Opener (Wurzel) oder ein Abzweig mit `parent_room_id` und `anchor_message_id`.

### Sichtbarkeit und Beitritt

| | auffindbar | Beitritt |
|---|---|---|
| **öffentlich** | ja | sofort |
| **auf Anfrage** | ja | Admin bestätigt |
| **nur Einladung** | nein | nur über Link |

Sichtbarkeit und Einladungslink sind **unabhängig**. Auch ein öffentlicher Space kann Links haben; bei „auf Anfrage" überspringt der Link die Freigabe. Nur die Rückrichtung ist zwingend: `invite_only` darf nirgends gelistet oder durchsuchbar sein.

### Schreibrecht — dritte Achse

`write_policy`: `all_members` oder `admins_only`.

`admins_only` ist der **Ankündigungskanal**. Der Unterschied zu Telegram: Der Opener ist gesperrt, **Abzweige sind offen**. Die Ankündigung bleibt unangetastet, die Rückfragen hängen daran statt sie zu zerreden.

---

## 4. Der AI-Layer

**Nicht dasselbe wie der Kanal.** Der Kanal ist chronologisch, der Layer ist thematisch. Sein Wert liegt genau darin, Nachrichten zusammenzuziehen, die zeitlich weit auseinanderliegen.

Belegt am echten Grillchat von 42Vienna


### Umsetzung

- **Grenzen über Embeddings** (Rolling Window, Cosinus-Distanz), **Zusammenfassungstext über LLM**. Bei API-Ausfall fällt nur der Text weg, die Struktur steht.
- **Mindestlänge pro Abschnitt** (z.B. 5 Nachrichten), sonst zerfällt ein lebhafter Chat in Dutzende Mini-Abschnitte.
- **Nie im Request-Pfad.** Schreiben setzt `summary_dirty`, ein Worker arbeitet Jobs ab.
- **Manuelle Einstiegspunkte funktionieren ohne KI.** Lang drücken → abzweigen. Der Agent liefert nur Vorschläge obendrauf.

### Fragen mit Status

Der Agent erkennt Fragen und setzt `open`, `answered` oder `dismissed`.

**Kritische Regel:** Agentenstatus ist immer nur ein Vorschlag. Jede menschliche Korrektur setzt `status_source = human` und wird nie wieder überschrieben. Die Klassifizierung wird danebenliegen — das muss eingeplant sein, nicht versteckt.

### Persönlicher Digest

„Deine Frage ist seit zwei Tagen offen." Das ist der stärkste Nutzen und der Grund, warum Teilnehmer an Accounts gebunden werden müssen.

---

## 5. Warum das nötig ist — Belege aus dem echten Chat

Analyse der `chat.md` (Community BBQ, 7 Wochen, ~40 Personen, 708 Zeilen):

**Fragen, die untergehen:**
- Stefan, 23.07: Sonnenbrille liegen gelassen → **nie beantwortet**, Thread endet
- Gregor, 21.07 10:29: Location-Update? → nie direkt beantwortet
- Gregor, 21.07 23:25: Wie viele Leute? → Antwort nach 10 Stunden
- Barbara, 11.07: Was bei Grillverbot? → Antwort nach 2 Tagen

**Dieselbe Frage mehrfach:**
- Stefan, 20.07: „hab vergessen, was ich mitbringen wollte"
- Lukas, 22.07: „gibt es eine Möglichkeit zu sehen, was ich beitrage?"

**Offene Zusage, 25 Tage unerledigt:**
- Mauro, 04.06: „ich frag nach und melde mich"
- Maksou, 29.06: „Gibt's Neuigkeiten?"
- Mauro: „shit, vergessen zu fragen"
- Ergebnis: kein Fleisch, das Event änderte sich

**Kernaussage:** Ein Gruppenchat ist unfreiwillig ein Projektmanagement-Werkzeug ohne jede Unterstützung dafür. Der Organisator macht die Koordinationsarbeit von Hand.

---

## 6. Bedienung

### Drei Ebenen, dieselbe Interaktion

Liste → Liste → Chat, mit Zurück-Navigation. Kein Graph — auf 380px unlesbar und mit dem Daumen kaum bedienbar.

### Aufklappen im Kanal

Tipp auf einen Post → die **Titelliste** der Abzweige klappt darunter auf (zwei Zeilen, keine Inhalte). Erst der zweite Tipp wechselt in den Raum.

- Immer nur ein Post gleichzeitig offen
- Zuletzt geöffneter bleibt gemerkt
- Im Chip: letzte Nachricht als Vorschau + Zahl ungelesener Beiträge

### Flache Liste, keine Baumansicht

Räume dürfen beliebig tief verzweigen, die Übersicht bleibt **einstufig**. Tiefe erscheint nur als Herkunftslabel („aus: …"), nie als Einrückung.

**Das ist die Lehre aus Google Wave**: Wave hatte Inline-Antworten an beliebiger Stelle und ist gescheitert, weil niemand wusste, wo eine Nachricht landet und wo man selbst gerade ist.

### Anker im Abzweig

Im Abzweig steht die Ankernachricht oben festgepinnt, ausgegraut, antippbar zum Zurückspringen. Ohne das geht der Kontext verloren — der Preis dafür, dass man in einen eigenen Raum wechselt statt einzurücken.

### Abzweige im Verlauf markieren

Im Chat zeigt eine dünne Trennlinie mit Titel und Nachrichtenzahl, wo abgezweigt wurde. Ohne diese Markierung laufen Leute an der halben Diskussion vorbei.

### Farbe

Farbe kodiert **Zustand**, nicht Typ. Offen / spät / gelöst. Der Typ (gesperrt, öffentlich) wird über Symbol und Beschriftung gezeigt. Amber ist durchgehend für „Ursprung" reserviert (Opener, Ankündigung, Anker).

### Pagination in beide Richtungen

Beim Sprung aus der Übersicht mitten in den Verlauf muss der Chat an beliebiger Position aufsetzen können. Muss von Anfang an im API-Design stehen.

---

## 7. Zugang

### Registrierte Nutzer
Volles Produkt: Benachrichtigungen, Digest, Abzweige erstellen, Rollen, Zugriff von überall.

### Gäste
- **Lesen braucht keine Identität.** Der Link ist der Ausweis. Kein Name, kein Eintrag in der Datenbank.
- **Name wird erst beim ersten Schreiben abgefragt.** Dann entsteht ein Teilnehmer mit Gast-Token im Cookie.
- Nur im eingeladenen Space, nur lesen und schreiben. Keine Abzweige, kein Status ändern, keine Rolle, kein Einladen.
- Keine Benachrichtigungen (kein Kanal dafür) — das ist der Hebel zur Registrierung.
- Zugang klebt am Gerät. Muss beim Beitritt gesagt werden.
- Namensdopplungen beim Anlegen prüfen.
- Rate Limit pro IP, Gastzugang pro Space abschaltbar, standardmäßig aus.
- 30 tage inaktivitaet loescht den gast nutzer

### Teilnehmer statt Nutzer

**Nachrichten zeigen immer auf `participant`, nie auf `user`.** Auch die von registrierten Nutzern.

Grund: Sonst muss jede Query, jede Anzeige, jede Berechtigungsprüfung zwei Fälle behandeln. So gibt es nur einen.

Ein Gast, der sich später registriert, verknüpft seinen Teilnehmer mit dem Account und übernimmt rückwirkend alle Beiträge. `claimed_at` einmal gesetzt ist endgültig.

Anzeigename gehört zum Teilnehmer, nicht zum Account — man kann pro Space anders heißen.

---

## 8. Bewusst verworfen

| Verworfen | Grund |
|---|---|
| **Graph-Navigation** | Auf 380px unlesbar, mit dem Daumen kaum bedienbar |
| **Chat-Import** | Kein etabliertes Nutzerverhalten. Einladungslink löst dasselbe besser |
| **Discord-Bot / Live-Spiegelung** | Diskussion spaltet sich, externe Abhängigkeit, Datenschutz-Sonderfall |
| **Zurückschreiben nach Discord** | APP-Badge bleibt, Reaktionen und Bezüge gehen verloren, zwingt in fremdes Datenmodell |
| **Integration statt eigenes Produkt** | Backend würde zum reinen Adapter, kaum eigene Substanz |
| **Abzweige nach unten ausklappen (Inhalte)** | Holt genau das Problem zurück, gegen das das Produkt antritt |
| **Baumansicht mit Einrückung** | Reddits Problem — mobil geht nach 4-5 Ebenen die Breite aus |

Ergebnis: kein Bot, keine Spiegelung, kein Parser, kein Datenschutz-Sonderfall. Übrig bleibt ein eigener Chat mit Einstiegspunkten, Agent-Layer und Einladungslinks.

---

## 9. Wettbewerb

| | was sie haben | was fehlt |
|---|---|---|
| **Zulip** | Topic-Modell, echte Communities, öffentlich lesbar, 1500+ Orgas gesponsert | Topics sind Labels beim Schreiben, keine Anker mitten im Chat. Kein Agent. Silos statt gemeinsamem Namensraum |
| **Google Wave** (tot) | Inline-Antworten an beliebiger Stelle | Gescheitert an Orientierungslosigkeit → deshalb flache Liste |
| **Reddit** | Verschachtelte Bäume | Keine Echtzeit, keine Zusammenfassungen, mobil geht die Breite aus |
| **ChatGPT / KnowTree** | Branching etabliert | Mensch↔KI, nicht Gruppe |
| **Otter / YouTube-Kapitel** | Segmentierung linearer Ströme | Keine Interaktion |
| **Discourse** | KI-Thread-Zusammenfassungen | Forum, kein Chat |

**Die Lücke:** Niemand kombiniert Gruppenchat + Verzweigung + agentische Segmentierung.

**Der verteidigbare Unterschied zu Zulip:** Zulip verlangt Struktur *vorher*, dieses Produkt erzeugt sie *danach*.

---

## 10. Datenschutz

Mit dem Wegfall des Imports stark entschärft. Was bleibt:

- Privat by default, kein Index über Raumgrenzen
- Kein Data-Mining über Nutzer hinweg
- Löschkaskade — auch Zusammenfassungen neu schreiben, wenn eine Person entfernt wird (der Name steht sonst noch drin, wenn die Nachricht weg ist)
- LLM-Anbieter als Auftragsverarbeiter in der Datenschutzerklärung benennen
- Keine personenbezogenen Auswertungen über Räume hinweg, keine Ranglisten, keine Antwortquoten pro Person

Der letzte Punkt ist bewusst: Ein Layer, der Fragen Personen zuordnet, erzeugt automatisch Nebenprodukte, die nach Leistungskontrolle aussehen. Technisch gar nicht erst ermöglichen.

---

## 11. Schema — Änderungen gegenüber dem ersten Entwurf

`section` war als zusammenhängender Bereich definiert (`from_message_id` / `to_message_id`). **Das ist falsch** für parallel laufende Themen.

Korrektur:

```
section          id, room_id, title, summary,
                 kind  'sequential' | 'thematic',
                 model_version, created_at

section_message  section_id, message_id, idx
                 PK(section_id, message_id)
```

Eine Section ist eine **Menge** von Nachrichten, kein Bereich. Eine Nachricht kann in mehreren Sections vorkommen (jemand fragt nach der Uhrzeit *und* erwähnt die Burger).

Übersicht sortiert nach der frühesten Nachricht einer Section. Karte zeigt die Zeitspanne: „Transport · 21.–22.07 · 7 Nachrichten".

Ebenfalls entfallen: `import_job`, `source_handle`, `source_external_id`, `claim_approved_by`.
Neu: `guest_token`, `guest_expires_at` bei `participant`. `write_policy` bei `space`.

Vollständiges Schema in `schema.md`, mit diesen Korrekturen zu lesen.

---

## 12. Module

Major (2): Framework Front+Back · Real-time WebSockets · User interaction · Standard User Management · Organization system · Advanced permissions · Modules of choice (Agent-Layer)

Minor (1): ORM · Advanced search · Notification system · Custom design system · Analytics dashboard · OAuth 2.0

---

## 13. Demo

**Reihenfolge:** erst ein öffentlicher Space (hier ist die Plattform), dann ein privater (hier ist das Problem).

**Kritisch:** Ein leerer oder kurzer Chat zeigt das Problem nicht. Es braucht 200+ Nachrichten mit mehreren parallel laufenden Themen, sonst fragt der Evaluator zu Recht, wozu das Ganze gut ist. Der Grillchat ist die Vorlage für realistische Seed-Daten.

**Fallback:** Der Agent-Layer braucht einen Weg ohne externe API, sonst steht die Demo, wenn das Netz zickt.

---

## 14. Offen

- API-Endpunkte, besonders Pagination in beide Richtungen
- Screens als Mockup
- Seed-Daten-Generator
- Wie der Agent zwischen `sequential` und `thematic` entscheidet
