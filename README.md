# fdb-tools

LEGO Universe FDB database tools. CLI converter and Qt6 GUI viewer.

> **Note:** This project was developed with significant AI assistance (Claude by Anthropic). All code has been reviewed and validated by the project maintainer, but AI-generated code may contain subtle issues. Contributions and reviews are welcome.

Part of the [LU-Rebuilt](https://github.com/LU-Rebuilt) project.

## Tools

### fdb_converter

Convert NetDevil FDB flat databases to SQLite.

```
fdb_converter <cdclient.fdb> <output.sqlite>
```

Produces a SQLite database with all tables, columns, and rows from the FDB. The output is compatible with DarkflameServer's CDServer.sqlite format.

### fdb_viewer

Qt6 GUI for browsing FDB and SQLite databases. Requires Qt6.

```
fdb_viewer [file.fdb or file.sqlite]
```

**Features:**
- Opens `.fdb` files directly (auto-converts to temporary SQLite in memory)
- Also opens `.sqlite` and `.db` files directly
- Table list with filter on the left dock
- Full table data view with sortable, resizable columns
- Row filtering via SQL WHERE clause (type filter and press Enter)
- Persistent last-open directory

**Keyboard shortcuts:**
- `Ctrl+O` — Open database

## Building

```bash
cmake -B build
cmake --build build -j$(nproc)
```

fdb_viewer is skipped automatically if Qt6 is not found.

For local development:

```bash
cmake -B build -DFETCHCONTENT_SOURCE_DIR_LU_ASSETS=/path/to/local/lu-assets \
               -DFETCHCONTENT_SOURCE_DIR_TOOL_COMMON=/path/to/local/tool-common
```

## Acknowledgments

Format parsers built from:
- **[lcdr/utils](https://github.com/lcdr/utils)** — Python FDB-to-SQLite reference implementation
- **[lcdr/lu_formats](https://github.com/lcdr/lu_formats)** — Kaitai Struct FDB format definition
- **[DarkflameServer](https://github.com/DarkflameServer/DarkflameServer)** — CDClient database schema reference
- **Ghidra reverse engineering** of the original LEGO Universe client binary

## License

[GNU Affero General Public License v3.0](https://www.gnu.org/licenses/agpl-3.0.html) (AGPLv3)

