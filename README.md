# 💻 IT Tracker — Evidencija Zarade

Profesionalni program za praćenje poslova, mušterija i zarade za IT preduzetnika.

---

## 📸 Funkcije

| Sekcija | Opis |
|---------|------|
| 📊 Dashboard | Pregled zarade ovog meseca, neplaćenih dugova, broja poslova, grafikoni |
| 📋 Evidencija poslova | Dodavanje, izmjena, brisanje poslova sa filterima |
| 👤 Mušterije | Upravljanje bazom mušterija |

### Što program prati:
- Koji posao si uradio, kome, kada i po kojoj cijeni
- Sate rada i cijenu po satu (automatski računa ukupno)
- Status plaćanja (plaćeno / čeka)
- Mesečna zarada sa grafikonom po mesecima
- Distribucija zarade po vrsti posla (pie chart)
- Filtri po godini, mesecu, mušteriji i vrsti posla

---

## 🚀 Brzi start — Kompajliranje

### Preduvjeti
- [Qt 6.x](https://www.qt.io/download-qt-installer) — instaliraj sa **MSVC 2019 64-bit** + **Qt Charts** modulom
- [CMake 3.16+](https://cmake.org/download/)
- Visual Studio 2019/2022 ili Build Tools

### Koraci

```bash
# 1. Kloniraj repo
git clone https://github.com/TVOJE_IME/ITTracker.git
cd ITTracker

# 2. Konfiguriši (zamijeni putanju sa svojom Qt instalacijom)
cmake -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ^
      -DCMAKE_PREFIX_PATH="C:\Qt\6.7.0\msvc2019_64"

# 3. Build
cmake --build build --config Release

# 4. Deploy Qt DLL-ova (da .exe radi bez Qt instalacije)
"C:\Qt\6.7.0\msvc2019_64\bin\windeployqt.exe" build\bin\ITTracker.exe

# 5. Pokreni
build\bin\ITTracker.exe
```

---

## ⚡ Automatski build via GitHub Actions

Program koristi **GitHub Actions** koji automatski pravi `.exe` svaki put kad pushuješ na `main`.

### Postavljanje:
1. Napravi novi GitHub repozitorij
2. Pushuj kod:
```bash
git init
git add .
git commit -m "Initial commit"
git remote add origin https://github.com/TVOJE_IME/ITTracker.git
git push -u origin main
```
3. Idi na tab **Actions** u GitHub-u — build se pokreće automatski
4. Kad završi (5-10 min), preuzmi ZIP sa `build\bin\ITTracker.exe` pod **Artifacts**

### Release (za verzionisanje):
```bash
git tag v1.0.0
git push origin v1.0.0
```
GitHub će automatski napraviti **Release** sa `.exe` paketom!

---

## 📦 Struktura projekta

```
ITTracker/
├── CMakeLists.txt          # Build konfiguracija
├── .github/
│   └── workflows/
│       └── build.yml       # GitHub Actions CI/CD
├── src/
│   ├── main.cpp            # Ulazna tačka
│   ├── mainwindow.h/cpp    # Glavni prozor sa sidebar-om
│   ├── database.h/cpp      # SQLite baza podataka
│   ├── dashboard.h/cpp     # Dashboard sa grafikonima
│   ├── clientspage.h/cpp   # Upravljanje mušterijama
│   ├── worklogpage.h/cpp   # Evidencija poslova
│   ├── jobdialog.h/cpp     # Dijalog za posao
│   ├── clientdialog.h/cpp  # Dijalog za mušteriju
│   └── style.h             # CSS tema (dark mode)
└── README.md
```

---

## 🗃️ Baza podataka

Podaci se čuvaju lokalno u SQLite fajlu na:
- **Windows:** `%APPDATA%\ITTracker\ITTracker\ittracker.db`
- **Linux/Mac:** `~/.local/share/ITTracker/ITTracker/ittracker.db`

---

## 🎨 Tehnologije

- **C++17** — programski jezik
- **Qt 6** — GUI framework (Widgets + Charts + SQL)
- **SQLite** — lokalna baza podataka (ugrađena u Qt)
- **CMake** — build sistem
- **GitHub Actions** — automatski build i release

---

## 📄 Licenca

MIT — slobodno koristi i prilagođavaj za vlastite potrebe.
