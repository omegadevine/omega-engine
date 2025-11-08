# Download and Run Instructions

## Downloading Omega Engine Demo v5

### From GitHub Releases

1. Go to [Releases](https://github.com/yourusername/omega-engine/releases)
2. Download `omega-engine-v5-standalone.exe`
3. Download `omega-engine-v5-standalone.exe.sha256` (checksum file)

### Security Warnings - What to Expect

**You WILL see security warnings** when downloading and running this executable. This is **normal** for independent software not signed by a commercial Certificate Authority.

#### During Download (Browser):
- Chrome: "This type of file can harm your computer" 
  - Click **"Keep"** or **"Keep anyway"**
- Edge: "This file isn't commonly downloaded"
  - Click **"..."** → **"Keep"**
- Firefox: Usually allows download without warning

#### When Running (Windows SmartScreen):
- "Windows protected your PC"
- Click **"More info"**
- Click **"Run anyway"**

### Why These Warnings Appear

This software is **self-signed**, not signed by a commercial Certificate Authority ($100-$400/year). The warnings don't mean the software is malicious - they mean:
- Windows doesn't recognize the publisher yet
- The executable needs to build "reputation" through downloads
- Microsoft wants commercial publishers to pay for certificates

### Verify Download Integrity

To verify your download hasn't been tampered with:

**Windows PowerShell:**
```powershell
# Compare this output with the .sha256 file
Get-FileHash -Algorithm SHA256 omega-engine-v5-standalone.exe
```

The hash should match exactly with the published checksum.

### Alternative: Build from Source

If you prefer to avoid warnings entirely:

```powershell
git clone https://github.com/yourusername/omega-engine.git
cd omega-engine
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

Then run: `build\Release\omega-engine.exe`

## Running the Demo

### Controls
- **WASD**: Move character (W=Up, A=Left, S=Down, D=Right)
- **Space**: Jump
- **ESC**: Exit game

### Gameplay
- Navigate through procedurally generated dungeons
- Avoid enemies (red squares)
- Collect healing potions (green circles)
- Find the key (yellow diamond) to unlock the exit
- Reach the exit portal (blue circle) to advance to next level

### System Requirements
- **OS**: Windows 10/11 (64-bit)
- **RAM**: 512 MB minimum
- **Graphics**: DirectX 9 compatible
- **Storage**: 50 MB

## Troubleshooting

### "VCRUNTIME140.dll missing"
- Download and install [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)

### Game crashes on startup
- Ensure graphics drivers are up to date
- Try running as Administrator
- Check that DirectX 9 is installed

### Performance issues
- Close other applications
- The game runs at 60 FPS by default
- Minimum 2 GHz CPU recommended

## Source Code

Full source code is available at: https://github.com/yourusername/omega-engine

## License

This software is licensed under the MIT License. See LICENSE file in the repository.

## Support

For issues, please open a GitHub issue or contact the developer.

---

**Note**: This is an early alpha version (v5) of the Omega Engine. Expect bugs and incomplete features.
