# Omega Engine Certificate Installation Guide

## What is this?

The Omega Engine executables are signed with a **self-signed certificate** to provide some verification that the files haven't been tampered with. However, since this isn't a commercially purchased certificate from a Certificate Authority (CA), you'll need to install our certificate on your machine to avoid security warnings.

## Why do I need to install it?

Without installing the certificate:
- Windows SmartScreen will show warnings when downloading
- Windows will show "Unknown Publisher" warnings when running executables
- The executables are still safe to run, but Windows doesn't recognize the certificate yet

After installing the certificate:
- Windows will recognize the signature
- Security warnings will be reduced
- You'll see "Omega Engine Developer" as the verified publisher

## How to Install the Certificate (Windows)

### Option 1: Install via PowerShell (Quick)
1. Open PowerShell as Administrator
2. Navigate to the omega-engine folder
3. Run:
   ```powershell
   Import-Certificate -FilePath ".\omega-engine-certificate.cer" -CertStoreLocation "Cert:\CurrentUser\Root"
   ```

### Option 2: Install via GUI (Manual)
1. Double-click `omega-engine-certificate.cer`
2. Click "Install Certificate..."
3. Select "Current User"
4. Choose "Place all certificates in the following store"
5. Click "Browse" and select "Trusted Root Certification Authorities"
6. Click "Next" and "Finish"
7. Click "Yes" when warned about installing the certificate

## Security Note

**This is a self-signed certificate**, which means:
- ✅ It verifies the files haven't been modified after signing
- ✅ It's completely free and doesn't require payment to any authority
- ❌ It doesn't prove identity to third parties (unlike commercial certificates)
- ❌ Other users will need to manually install it on their machines

Only install this certificate if you trust the source (the official Omega Engine repository or a trusted developer).

## For Developers

The certificate is valid for 3 years and was created specifically for signing Omega Engine builds. If you're building from source yourself, you can create your own certificate or use the existing one if you have access to the private key (stored in your Windows certificate store).

## Questions?

If you have concerns about security or certificate installation, please open an issue on the GitHub repository.
