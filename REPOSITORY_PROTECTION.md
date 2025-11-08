# Repository Protection Guide

## Critical Protection Measures

### 1. Enable Branch Protection (Manual - GitHub Settings)
Go to: `Settings > Branches > Add rule` for `main`:
- ✅ Require pull request reviews before merging
- ✅ Require status checks to pass (CI, CodeQL)
- ✅ Require branches to be up to date
- ✅ Require signed commits (recommended)
- ✅ Include administrators
- ✅ Restrict who can push to matching branches
- ✅ Do not allow force pushes
- ✅ Do not allow deletions

### 2. Enable Security Features (Manual - GitHub Settings)
Go to: `Settings > Security`:
- ✅ Private vulnerability reporting
- ✅ Dependency graph
- ✅ Dependabot alerts
- ✅ Dependabot security updates
- ✅ Code scanning (CodeQL)
- ✅ Secret scanning

### 3. Install Pre-commit Hook
```bash
chmod +x .git/hooks/pre-commit
```

### 4. Regular Backups
- Automated weekly backups via GitHub Actions
- Download backup bundles from Actions artifacts
- Keep local copies of important branches

### 5. Commit Signing (Highly Recommended)
```bash
git config --global commit.gpgsign true
git config --global user.signingkey YOUR_GPG_KEY_ID
```

## Best Practices
1. Never commit credentials or secrets
2. Review all changes before pushing
3. Use meaningful commit messages
4. Keep dependencies updated via Dependabot
5. Monitor CodeQL scan results
6. Regularly review repository access permissions
7. Enable 2FA on your GitHub account
8. Use SSH keys or PAT for authentication, not passwords

## Recovery Procedures
If repository corruption occurs:
1. Use backup bundles from Actions artifacts
2. Clone fresh copy: `git clone --mirror`
3. Check reflog: `git reflog`
4. Contact GitHub Support for severe issues
