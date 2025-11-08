# Security Verification for Omega Engine Executables

## ⚠️ Windows SmartScreen Warnings

When downloading and running executables from this repository, you may encounter Windows SmartScreen warnings. This is **normal and expected** for unsigned executables.

### Why This Happens

1. **Unsigned Executables**: These executables are not signed with a commercial code signing certificate ($180-$500/year)
2. **New Files**: Windows SmartScreen hasn't built reputation data for these files
3. **Direct Downloads**: Files downloaded from GitHub are treated more cautiously

### This Does NOT Mean the Files Are Unsafe

- All source code is available in this repository
- You can build from source yourself
- SHA256 checksums are provided for verification
- The project is open source and transparent

## How to Safely Run the Executables

### Option 1: Verify and Run (Recommended)

1. **Verify the SHA256 hash** (see below)
2. When Windows SmartScreen warning appears:
   - Click **"More info"**
   - Click **"Run anyway"**
3. If your antivirus blocks it, add an exception

### Option 2: Build from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/omega-engine.git
cd omega-engine

# Build with CMake (requires Visual Studio/MSVC)
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Or use the provided batch file
cd ..
build_all_static.bat
```

### Option 3: Check the Source Code

All executable source code is in the repository:
- `dungeon_quest_v5_balanced.exe` - Built from source in repo
- `dungeon_quest_v4_lightning_static.exe` - Built from source in repo

Review the code yourself before running.

## SHA256 Checksums

Verify file integrity by comparing hashes:

```
Algorithm: SHA256
Hash: 3FFBDB0E7443E3D258066612744ACD99E918FCCB51884645515F85670AA884CA
File: dungeon_quest_v5_STANDALONE.exe

Algorithm: SHA256
Hash: 3FFBDB0E7443E3D258066612744ACD99E918FCCB51884645515F85670AA884CA
File: dungeon_quest_v5_balanced.exe

Algorithm: SHA256
Hash: E98D8CF560C52449116539AEE681311F9A48227C417DA79535A0FA4C57455901
File: dungeon_quest_v4_lightning_static.exe
```

### How to Verify Checksums

**On Windows (PowerShell):**
```powershell
Get-FileHash -Algorithm SHA256 "dungeon_quest_v5_STANDALONE.exe"
```

**On Linux/Mac:**
```bash
sha256sum dungeon_quest_v5_STANDALONE.exe
```

Compare the output with the checksums above. They should match exactly.

## Security Best Practices

### What We Do

✅ **Open Source**: All code is publicly visible  
✅ **Provide Checksums**: Verify file integrity  
✅ **Build Instructions**: You can compile yourself  
✅ **Transparent**: No hidden code or obfuscation  
✅ **Git Signed Commits**: Commits are traceable  

### What We Don't Do (Yet)

❌ **Code Signing Certificate**: Requires $180-500/year investment  
❌ **Windows Store Distribution**: Requires Microsoft certification  
❌ **Installer Package**: Would require additional signing  

## For Advanced Users

### Scan with VirusTotal

Upload any executable to [VirusTotal.com](https://www.virustotal.com/) to scan with 70+ antivirus engines.

### Inspect with Process Monitor

Use [Process Monitor](https://docs.microsoft.com/en-us/sysinternals/downloads/procmon) to see exactly what the executable does.

### Decompile and Inspect

Use tools like:
- IDA Free
- Ghidra
- PE Explorer

To inspect the executable structure.

## Future Plans

When the project reaches maturity and has funding:
- Purchase a code signing certificate
- Sign all releases officially
- Eliminate SmartScreen warnings

## Questions?

If you have security concerns, please:
1. Open an issue on GitHub
2. Review the source code
3. Build from source yourself
4. Ask questions in Discussions

## Trust Model

This is an **open source project** where:
- You can see all the code
- You can build it yourself
- You can verify checksums
- You can audit everything

The best security is **transparency**, not just a signature.

---

**Remember**: A code signing certificate only means "someone paid money" - it doesn't guarantee safety. Open source code that you can review is often more trustworthy than closed-source signed binaries.
