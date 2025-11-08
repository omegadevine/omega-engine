# Self-Sign Omega Engine Executables
# Run this script as Administrator in PowerShell

Write-Host "=== Omega Engine Self-Signing Tool ===" -ForegroundColor Cyan
Write-Host ""

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "ERROR: This script must be run as Administrator!" -ForegroundColor Red
    Write-Host "Right-click PowerShell and select 'Run as Administrator'" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "Step 1: Creating self-signed certificate..." -ForegroundColor Green

try {
    # Create self-signed certificate
    $cert = New-SelfSignedCertificate `
        -Type CodeSigningCert `
        -Subject "CN=Omega Engine Development" `
        -KeyExportPolicy Exportable `
        -CertStoreLocation Cert:\CurrentUser\My `
        -NotAfter (Get-Date).AddYears(5) `
        -KeyLength 2048 `
        -KeyAlgorithm RSA `
        -HashAlgorithm SHA256
    
    Write-Host "✓ Certificate created successfully" -ForegroundColor Green
    Write-Host "  Thumbprint: $($cert.Thumbprint)" -ForegroundColor Gray
    
    # Export certificate for distribution
    $certPath = Join-Path $PSScriptRoot "OmegaEngineCert.cer"
    Export-Certificate -Cert $cert -FilePath $certPath | Out-Null
    Write-Host "✓ Certificate exported to: $certPath" -ForegroundColor Green
    
    Write-Host ""
    Write-Host "Step 2: Installing certificate to Trusted Root..." -ForegroundColor Green
    
    # Import to Trusted Root
    $store = New-Object System.Security.Cryptography.X509Certificates.X509Store("Root", "CurrentUser")
    $store.Open("ReadWrite")
    $store.Add($cert)
    $store.Close()
    
    Write-Host "✓ Certificate installed to Trusted Root" -ForegroundColor Green
    
    Write-Host ""
    Write-Host "Step 3: Checking for signtool.exe..." -ForegroundColor Green
    
    # Find signtool.exe
    $signtoolPaths = @(
        "C:\Program Files (x86)\Windows Kits\10\bin\*\x64\signtool.exe",
        "C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe",
        "C:\Program Files (x86)\Microsoft SDKs\Windows\*\bin\*\signtool.exe"
    )
    
    $signtool = $null
    foreach ($path in $signtoolPaths) {
        $found = Get-ChildItem $path -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) {
            $signtool = $found.FullName
            break
        }
    }
    
    if (-not $signtool) {
        Write-Host "⚠ signtool.exe not found. Please install Windows SDK." -ForegroundColor Yellow
        Write-Host "  Download from: https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/" -ForegroundColor Gray
        Write-Host ""
        Write-Host "Certificate created but executables not signed." -ForegroundColor Yellow
        Read-Host "Press Enter to exit"
        exit 0
    }
    
    Write-Host "✓ Found signtool: $signtool" -ForegroundColor Green
    
    Write-Host ""
    Write-Host "Step 4: Signing executables..." -ForegroundColor Green
    
    # Find all .exe files in current directory
    $exeFiles = Get-ChildItem -Path $PSScriptRoot -Filter "*.exe"
    
    if ($exeFiles.Count -eq 0) {
        Write-Host "⚠ No .exe files found in current directory" -ForegroundColor Yellow
        Read-Host "Press Enter to exit"
        exit 0
    }
    
    foreach ($exe in $exeFiles) {
        Write-Host "  Signing: $($exe.Name)..." -ForegroundColor Cyan
        
        $signArgs = @(
            "sign",
            "/n", "Omega Engine Development",
            "/t", "http://timestamp.digicert.com",
            "/fd", "SHA256",
            "/v",
            $exe.FullName
        )
        
        $result = & $signtool $signArgs 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  ✓ $($exe.Name) signed successfully" -ForegroundColor Green
        } else {
            Write-Host "  ✗ Failed to sign $($exe.Name)" -ForegroundColor Red
            Write-Host "  Error: $result" -ForegroundColor Gray
        }
    }
    
    Write-Host ""
    Write-Host "=== Signing Complete ===" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Your executables are now signed with a self-signed certificate." -ForegroundColor Green
    Write-Host ""
    Write-Host "⚠ Important Notes:" -ForegroundColor Yellow
    Write-Host "- On YOUR computer, the executables will now run without warnings" -ForegroundColor Gray
    Write-Host "- On OTHER computers, users must install OmegaEngineCert.cer first" -ForegroundColor Gray
    Write-Host "- For commercial distribution, consider purchasing a proper code signing certificate" -ForegroundColor Gray
    Write-Host ""
    
} catch {
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host ""
}

Read-Host "Press Enter to exit"
