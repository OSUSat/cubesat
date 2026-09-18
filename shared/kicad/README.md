# OSUSat Shared KiCad Assets & Templates

This directory contains shared KiCad symbols, footprints, 3D models, design rules, library tables, and project templates for the **OSUSat CubeSat** hardware ecosystem.

---

## Directory Contents

- **`templates/`**: Project templates for standard board configurations (e.g. `test_board` expansion cards).
- **`symbols/`**: Shared subsystem component symbol libraries (`OSUSAT_EPS`, `OSUSAT_OBC`, `OSUSAT_Payload`, etc.).
- **`footprints/`**: Shared PCB footprint libraries (`OSUSAT_Connectors.pretty`, `OSUSAT_ICs.pretty`, etc.).
- **`3dmodels/`**: 3D STEP/WRL CAD models for components and mechanical alignments.
- **`rules/`**: Pre-defined DRC rules and PCB stackup definitions.

---

## Adding Templates to KiCad via Symlinking

Symlinking links the shared repository template directory directly into your local KiCad environment. Any updates pulled from git will be available in KiCad without copying files manually.

### 🐧 Linux Instructions

1. Open a terminal and ensure your KiCad template directory exists:
   ```bash
   mkdir -p ~/.local/share/kicad/10.0/template
   ```
   *(Replace `10.0` with your installed KiCad version: `10.0`, `9.0`, `8.0`, etc.)*

2. Create a symbolic link pointing to the repository template:
   ```bash
   ln -sf /path/to/cubesat/repo/shared/kicad/templates/test_board ~/.local/share/kicad/10.0/template/test_board
   ```

---

### 🪟 Windows Instructions

#### Option 1: PowerShell
Run PowerShell and execute:

```powershell
# ensure user template directory exists
New-Item -ItemType Directory -Force -Path "$env:APPDATA\kicad\10.0\template"

# create directory symbolic link
New-Item -ItemType SymbolicLink -Path "$env:APPDATA\kicad\10.0\template\test_board" -Target "C:\path\to\cubesat\repo\shared\kicad\templates\test_board"
```
*(Replace `10.0` and `C:\path\to\cubesat` with your installed KiCad version and repo path).*

#### Option 2: Command Prompt (`cmd.exe`)
Open Command Prompt (Run as Administrator):

```cmd
mkdir "%APPDATA%\kicad\10.0\template"
mklink /D "%APPDATA%\kicad\10.0\template\test_board" "C:\path\to\cubesat\shared\kicad\templates\test_board"
```

---

### ⚙️ Alternative: Configure Path in KiCad GUI

1. Open KiCad.
2. Navigate to **Preferences → Configure Paths...**
3. Under **Environment Variables**, add `KICAD_USER_TEMPLATE_DIR`:
   - **Name**: `KICAD_USER_TEMPLATE_DIR`
   - **Path**: Absolute path to `.../cubesat/shared/kicad/templates`
4. Click **OK**.

---

## 🚀 Creating a Board from Template

1. Open KiCad.
2. Select **File → New Project from Template...** (or `Ctrl+N`).
3. Click the **User Templates** tab.
4. Select **OSUSat Backplane Test Board Template**.
5. Choose your destination directory and enter your project name (e.g. `payload_card_v1`).
6. Click **Save**.
