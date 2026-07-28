# `.vscode/`

Dieser Ordner enthält meine persönliche Editor- und Shell-Konfiguration für dieses Projekt. Er ist **kein Bestandteil des Builds** und wird für `make`/`ircserv` nicht benötigt — er sammelt lediglich Settings, Snippets und Dotfile-Fragmente, die ich beim Arbeiten an diesem Repo verwende.

Ein Teil der Dateien wird von VSCode automatisch geladen, ein anderer Teil sind reine Quelldateien, die von Hand an einen anderen Ort im System kopiert werden müssen. Das ist unten pro Datei vermerkt.

## Automatisch von VSCode geladen

| Datei | Zweck |
|---|---|
| `settings.json` | Workspace-Settings: echte Tabs statt Spaces (Breite 4), 80-Zeichen-Ruler, kein Autoformat für C++, C/C++-Extension mit aktivierter `clang-tidy`-Analyse, 42-Header-Plugin (Username/E-Mail für den 42-Kommentarkopf), erzwungenes High-Contrast-Theme mit violetten Akzentfarben, Terminal-Shell-Integration bewusst deaktiviert, Formatter für JS/TS/Svelte (Prettier). |
| `keybindings.json` | Workspace-Keybindings, u. a. `Ctrl+Alt+Pfeiltasten` zum Fokussieren von Explorer/Terminal. Diese Kombination kollidiert mit GNOMEs Workspace-Umschalter — siehe dazu `bash/gsettings_bash`. |

## Code-Snippets (müssen manuell kopiert werden)

VSCode lädt Snippets **nicht** automatisch aus dem `.vscode`-Ordner eines Projekts. Diese beiden Dateien sind daher nur die Quelle — der jeweilige Dateikopf verweist bereits auf das Ziel:

| Datei | Ziel | Inhalt |
|---|---|---|
| `c.json` | `~/.config/Code/User/snippets/c.json` | 42er-C-Snippets: `while`, `for`, `if`/`elif`, `main`/`av`, `typedef struct`, `calloc` + Null-Check, `include`, `define`, `ifndef`-Guard, `switch` u. a. |
| `cpp.json` | `~/.config/Code/User/snippets/cpp.json` | Analoge Snippets für C++. |

## Bash-Konfiguration (`bash/`)

Modulare Shell-Config-Bausteine. `bashrc` ist die fertige Zusammensetzung der übrigen Fragmente und für `~/.bashrc` gedacht — die anderen Dateien sind einzelne Bestandteile bzw. Installationsnotizen daraus.

| Datei | Zweck |
|---|---|
| `bashrc` | Vollständige `~/.bashrc`: Ubuntu-Standardteil + eigener Prompt mit Exit-Status-/Signal-Anzeige (z. B. `SEGFAULT`, `SIGINT`), Git-Branch im Prompt, LS_COLORS-Overrides und `ble.sh`-Einbindung. |
| `ps1_prompt` | Nur das Prompt-Fragment (Exit-Status-Icon + `PS1`/`PS2`) aus `bashrc`, isoliert zum Wiederverwenden. |
| `LS_COLORS_nobara` | LS_COLORS-Override-Fragment, Variante für Nobara Linux. |
| `LS_COLOURS_ubuntu` | LS_COLORS-Override-Fragment, Variante für Ubuntu. |
| `gsettings_bash` | `gsettings`-Befehle, die GNOMEs `Ctrl+Alt+Pfeiltasten`-Workspace-Umschaltung deaktivieren, damit die VSCode-Keybindings aus `keybindings.json` nicht kollidieren. Muss manuell ausgeführt werden. |
| `bash_line_editor` | Installationsnotizen und `.blerc`-Konfiguration für [ble.sh](https://github.com/akinomyoga/ble.sh) (Fish-Style Ghost-Text-Autovervollständigung in Bash). |

## Hinweis

Alles hier sind persönliche Präferenzen. Wer dieses Repo klont, muss nichts davon übernehmen — `ircserv` baut und läuft unabhängig davon.