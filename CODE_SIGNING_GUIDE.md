# Code Signing Guide for Omega Engine

## Security Warning Issues

Windows SmartScreen and antivirus software flag unsigned executables as potentially unsafe. Here's how to resolve this:

## Option 1: Self-Signed Certificate (Free, Immediate)

### Create a Self-Signed Certificate:
```powershell
# Run PowerShell as Administrator
$cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject "CN=Omega Engine Development" -KeyExportPolicy Exportable -CertStoreLocation Cert:\CurrentUser\My -NotAfter (Get-Date).AddYears(5)

# Export certificate
Export-Certificate -Cert $cert -FilePath "OmegaEngineCert.cer"
```

### Sign Your Executables:
```powershell
# Sign the executable
signtool sign /n "Omega Engine Development" /t http://timestamp.digicert.com /fd SHA256 "dungeon_quest_v5_STANDALONE.exe"
```

### Install Certificate (Users need to do this):
1. Double-click `OmegaEngineCert.cer`
2. Click "Install Certificate"
3. Select "Local Machine" (requires admin)
4. Place in "Trusted Root Certification Authorities"

**Note:** Self-signed certificates will still show warnings on other computers until users install the certificate.

## Option 2: Commercial Code Signing Certificate (Recommended for Distribution)

### Providers:
- **DigiCert** ($474/year) - Industry standard
- **Sectigo** ($179/year) - Budget-friendly
- **GlobalSign** ($249/year) - Good reputation

### Benefits:
- No warnings on user computers
- Builds trust and reputation over time
- Required for serious distribution

### Process:
1. Purchase certificate from provider
2. Verify your identity (business or individual)
3. Receive certificate file (.pfx)
4. Sign executables:
```powershell
signtool sign /f "certificate.pfx" /p "password" /tr http://timestamp.digicert.com /td SHA256 /fd SHA256 "your_executable.exe"
```

## Option 3: Build Reputation (Free, Takes Time)

Even unsigned executables become trusted over time if:
- Many users download and run them without issues
- Windows SmartScreen learns the file is safe
- This can take weeks/months and hundreds of downloads

## Option 4: Provide Source Code + Build Instructions

For open-source projects:
- Users can build from source
- No security warnings when self-compiled
- Most transparent option

## Immediate Workarounds for Users

### Method 1: Windows SmartScreen Override
1. When warning appears, click "More info"
2. Click "Run anyway"

### Method 2: Antivirus Exception
Add the executable to your antivirus whitelist/exceptions

### Method 3: Verify Hash
Provide SHA256 hashes in README for users to verify:
```powershell
Get-FileHash -Algorithm SHA256 "dungeon_quest_v5_STANDALONE.exe"
```

## Current Implementation

For now, I recommend:

1. **Document the expected behavior** in README
2. **Provide SHA256 hashes** for verification
3. **Include build-from-source instructions**
4. **For serious distribution**: Invest in a code signing certificate

## Additional Security Measures

### Add Digital Signatures to Repository
- Tag releases with GPG signatures
- Sign commits with GPG

### Add Checksum File
```powershell
Get-FileHash -Algorithm SHA256 *.exe | Format-List | Out-File CHECKSUMS.txt
```

### Add VirusTotal Scanning
- Upload executables to VirusTotal
- Share scan results link in README

## Notes

- **Self-signed certificates** work but require user trust
- **Commercial certificates** are the professional solution
- **Building reputation** is free but slow
- **Open source** with build instructions is most transparent

For a hobby/development project, self-signed certificates or providing source code is sufficient. For commercial distribution, invest in a proper code signing certificate.
