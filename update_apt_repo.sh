#!/usr/bin/env bash
set -euo pipefail

# ==============================================================================
# Script de mise à jour du Dépôt APT Q4WIN10 & GitHub Pages
# ==============================================================================

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
PAGES_BRANCH="gh-pages"

need_cmd() {
    command -v "$1" >/dev/null 2>&1 || {
        echo "Error: missing required command: $1" >&2
        exit 1
    }
}

need_cmd dpkg-scanpackages
need_cmd apt-ftparchive
need_cmd git

echo "=================================================="
echo " Q4WIN10 APT Repository & GitHub Pages Sync"
echo "=================================================="

# Ensure we have deb and qsi packages
DEB_FILES=($(find "$REPO_DIR" -maxdepth 1 -name "tde-win-deco-q4win10_*_amd64.deb" | sort -V -r))
if [ ${#DEB_FILES[@]} -eq 0 ]; then
    echo "Error: No tde-win-deco-q4win10_*_amd64.deb package found in $REPO_DIR." >&2
    echo "Please run ./create_deb.sh first." >&2
    exit 1
fi

QSI_FILES=($(find "$REPO_DIR" -maxdepth 1 -name "setup_tde-win-deco-q4win10_*.qsi" | sort -V -r))

PAGES_DIR=$(mktemp -d -t q4win10-gh-pages-XXXXXX)
echo "Staging in temporary directory: $PAGES_DIR"

REMOTE_URL="$(git -C "$REPO_DIR" remote get-url origin)"

# Clone or checkout gh-pages into temp directory
git clone --branch "$PAGES_BRANCH" --single-branch "$REMOTE_URL" "$PAGES_DIR" 2>/dev/null || {
    echo "Creating new orphan gh-pages branch in temp directory..."
    git init "$PAGES_DIR"
    (
        cd "$PAGES_DIR"
        git checkout --orphan "$PAGES_BRANCH"
        git remote add origin "$REMOTE_URL"
    )
}

# Structure pool and dists directories for standard APT repo
POOL_DIR="$PAGES_DIR/pool/main/t/tde-win-deco-q4win10"
DISTS_DIR="$PAGES_DIR/dists/stable/main/binary-amd64"
mkdir -p "$POOL_DIR"
mkdir -p "$DISTS_DIR"

# Copy all deb packages into pool
for deb in "${DEB_FILES[@]}"; do
    echo "  -> Added DEB: $(basename "$deb")"
    cp -a "$deb" "$POOL_DIR/"
done

# Copy latest QSI to root of pages
for qsi in "${QSI_FILES[@]}"; do
    echo "  -> Added QSI: $(basename "$qsi")"
    cp -a "$qsi" "$PAGES_DIR/"
done

# Generate Packages & Packages.gz index files
echo "Generating Packages index..."
(
    cd "$PAGES_DIR"
    dpkg-scanpackages --multiversion --arch amd64 pool/main > "$DISTS_DIR/Packages"
    gzip -9 -c "$DISTS_DIR/Packages" > "$DISTS_DIR/Packages.gz"
)

# Generate Release file
echo "Generating Release manifest..."
apt-ftparchive \
  -o APT::FTPArchive::Release::Origin="Q4WIN10" \
  -o APT::FTPArchive::Release::Label="Q4WIN10 Window Decoration APT Repository" \
  -o APT::FTPArchive::Release::Suite="stable" \
  -o APT::FTPArchive::Release::Codename="stable" \
  -o APT::FTPArchive::Release::Architectures="amd64" \
  -o APT::FTPArchive::Release::Components="main" \
  -o APT::FTPArchive::Release::Description="APT Repository for Q4WIN10 Window Decoration (Trinity Desktop)" \
  release "$PAGES_DIR/dists/stable" > "$PAGES_DIR/dists/stable/Release"

# Copy assets (logo, favicon, screenshots, etc.)
if [ -f "$REPO_DIR/tdewin_big.png" ]; then
    cp -a "$REPO_DIR/tdewin_big.png" "$PAGES_DIR/"
elif [ -f "$REPO_DIR/_ressources/tdewin_big.png" ]; then
    cp -a "$REPO_DIR/_ressources/tdewin_big.png" "$PAGES_DIR/tdewin_big.png"
fi

if [ -f "$REPO_DIR/favicon.png" ]; then
    cp -a "$REPO_DIR/favicon.png" "$PAGES_DIR/"
elif [ -f "$REPO_DIR/tdewin_128_16.png" ]; then
    cp -a "$REPO_DIR/tdewin_128_16.png" "$PAGES_DIR/favicon.png"
fi

if [ -d "$REPO_DIR/screenshots" ]; then
    mkdir -p "$PAGES_DIR/screenshots"
    cp -a "$REPO_DIR/screenshots"/* "$PAGES_DIR/screenshots/"
fi

# Create .nojekyll to prevent GitHub Pages Jekyll processing
touch "$PAGES_DIR/.nojekyll"

# Find latest file names and version for HTML
LATEST_VERSION=$(grep '#define Q4WIN10_VERSION "' "$REPO_DIR/q4win10_version.h" 2>/dev/null | sed -E 's/.*"([^"]+)".*/\1/' || echo "2.0.1")
LATEST_DEB_NAME=$(basename "${DEB_FILES[0]}")
LATEST_QSI_NAME=""
if [ ${#QSI_FILES[@]} -gt 0 ]; then
    LATEST_QSI_NAME=$(basename "${QSI_FILES[0]}")
fi

echo "Generating index.html..."
cat << EOF > "$PAGES_DIR/index.html"
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Q4WIN10 Window Decoration v${LATEST_VERSION} - APT Repository</title>
  <link rel="icon" type="image/png" href="favicon.png">
  <meta name="description" content="Official APT Repository and download portal for Q4WIN10 - Lightweight, flat Windows 10 & 11 inspired window decoration plugin for Trinity Desktop Environment (TDE).">
  <style>
    :root {
      --bg: #12141a;
      --card-bg: #1c1f2b;
      --card-hover: #222738;
      --accent: #38bdf8;
      --accent-grad: linear-gradient(135deg, #0284c7, #38bdf8);
      --text: #e2e8f0;
      --text-muted: #94a3b8;
      --code-bg: #0f1117;
      --border: #2e364f;
      --radius: 12px;
      --radius-sm: 8px;
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }

    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      background-color: var(--bg);
      color: var(--text);
      line-height: 1.6;
      padding: 40px 20px;
    }

    .container {
      max-width: 840px;
      margin: 0 auto;
    }

    header {
      text-align: center;
      margin-bottom: 40px;
    }

    .logo {
      width: 110px;
      height: 110px;
      margin-bottom: 16px;
      filter: drop-shadow(0 8px 24px rgba(56, 189, 248, 0.4));
      border-radius: 18px;
      transition: transform 0.3s cubic-bezier(0.34, 1.56, 0.64, 1);
    }

    .logo:hover {
      transform: scale(1.08) rotate(3deg);
    }

    .badge-group {
      display: flex;
      justify-content: center;
      gap: 10px;
      margin-bottom: 12px;
      flex-wrap: wrap;
    }

    .badge {
      display: inline-block;
      padding: 4px 14px;
      font-size: 0.85rem;
      font-weight: 600;
      color: #fff;
      background: var(--accent-grad);
      border-radius: 20px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }

    .badge-green {
      background: linear-gradient(135deg, #15803d, #22c55e);
    }

    .badge-purple {
      background: linear-gradient(135deg, #6366f1, #a855f7);
    }

    .version-pill {
      display: inline-block;
      font-size: 1.1rem;
      font-weight: 600;
      color: #38bdf8;
      background: rgba(56, 189, 248, 0.12);
      border: 1px solid rgba(56, 189, 248, 0.35);
      padding: 2px 12px;
      border-radius: 20px;
      vertical-align: middle;
      margin-left: 8px;
    }

    h1 {
      font-size: 2.4rem;
      font-weight: 700;
      margin-bottom: 8px;
      display: flex;
      align-items: center;
      justify-content: center;
    }

    p.lead {
      font-size: 1.1rem;
      color: var(--text-muted);
      max-width: 680px;
      margin: 0 auto;
    }

    .card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 24px;
      margin-bottom: 24px;
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
    }

    h2 {
      font-size: 1.3rem;
      margin-bottom: 14px;
      display: flex;
      align-items: center;
      gap: 10px;
      color: #fff;
    }

    /* Terminal & Code snippet box */
    .code-container {
      position: relative;
      margin-top: 10px;
    }

    pre {
      background: var(--code-bg);
      border: 1px solid var(--border);
      border-radius: var(--radius-sm);
      padding: 16px;
      padding-right: 80px;
      overflow-x: auto;
      font-family: "Courier New", Courier, monospace;
      font-size: 0.92rem;
      color: #38bdf8;
      line-height: 1.6;
    }

    .copy-btn {
      position: absolute;
      top: 12px;
      right: 12px;
      background: rgba(255, 255, 255, 0.08);
      border: 1px solid rgba(255, 255, 255, 0.18);
      color: var(--text);
      padding: 5px 12px;
      border-radius: 6px;
      font-size: 0.8rem;
      cursor: pointer;
      transition: all 0.2s;
    }

    .copy-btn:hover {
      background: var(--accent);
      color: #fff;
      border-color: var(--accent);
    }

    /* Downloads Grid */
    .downloads-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
      gap: 16px;
      margin-top: 16px;
    }

    .download-card {
      background: #141722;
      border: 1px solid var(--border);
      border-radius: var(--radius-sm);
      padding: 18px;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      transition: all 0.2s ease;
    }

    .download-card:hover {
      transform: translateY(-2px);
      border-color: #38bdf8;
      background: var(--card-hover);
    }

    .download-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin-bottom: 8px;
    }

    .download-title {
      font-size: 1.05rem;
      font-weight: 700;
      color: #fff;
    }

    .download-tag {
      font-size: 0.72rem;
      font-weight: 600;
      padding: 2px 8px;
      border-radius: 12px;
      background: rgba(56, 189, 248, 0.15);
      color: #38bdf8;
      border: 1px solid rgba(56, 189, 248, 0.3);
    }

    .download-desc {
      font-size: 0.85rem;
      color: var(--text-muted);
      margin-bottom: 14px;
      flex-grow: 1;
    }

    .btn-download {
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      background: var(--accent-grad);
      color: #fff;
      text-decoration: none;
      font-weight: 600;
      font-size: 0.9rem;
      padding: 10px 16px;
      border-radius: var(--radius-sm);
      transition: opacity 0.2s;
    }

    .btn-download:hover {
      opacity: 0.9;
    }

    /* Key Features Grid */
    .features-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(230px, 1fr));
      gap: 16px;
      margin-top: 16px;
    }

    .feature-item {
      background: #141722;
      border: 1px solid var(--border);
      border-radius: var(--radius-sm);
      padding: 16px;
    }

    .feature-icon {
      font-size: 1.5rem;
      margin-bottom: 8px;
      display: inline-block;
    }

    .feature-title {
      font-size: 0.98rem;
      font-weight: 600;
      color: #fff;
      margin-bottom: 4px;
    }

    .feature-text {
      font-size: 0.84rem;
      color: var(--text-muted);
      line-height: 1.5;
    }

    /* Screenshots Grid */
    .screenshots-grid {
      display: flex;
      justify-content: center;
      margin-top: 16px;
    }

    .screenshot-thumb {
      max-width: 100%;
      border: 1px solid var(--border);
      border-radius: var(--radius-sm);
      overflow: hidden;
      cursor: pointer;
      transition: transform 0.2s, border-color 0.2s;
    }

    .screenshot-thumb:hover {
      transform: scale(1.02);
      border-color: #38bdf8;
    }

    .screenshot-thumb img {
      width: 100%;
      height: auto;
      display: block;
    }

    /* Modal */
    .modal {
      display: none;
      position: fixed;
      z-index: 1000;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: rgba(0, 0, 0, 0.85);
      align-items: center;
      justify-content: center;
      backdrop-filter: blur(4px);
    }

    .modal.active {
      display: flex;
    }

    .modal img {
      max-width: 90%;
      max-height: 90%;
      border-radius: 8px;
      box-shadow: 0 10px 40px rgba(0, 0, 0, 0.8);
    }

    .modal-close {
      position: absolute;
      top: 20px;
      right: 30px;
      color: #fff;
      font-size: 2rem;
      cursor: pointer;
      user-select: none;
    }

    /* Footer */
    footer {
      text-align: center;
      margin-top: 40px;
      font-size: 0.85rem;
      color: var(--text-muted);
    }

    footer a {
      color: var(--accent);
      text-decoration: none;
    }

    footer a:hover {
      text-decoration: underline;
    }

    .footer-links {
      margin-top: 10px;
      font-size: 0.8rem;
    }
  </style>
</head>
<body>

  <div class="container">

    <!-- Header -->
    <header>
      <img src="tdewin_big.png" alt="Q4WIN10 Logo" class="logo">
      <div class="badge-group">
        <span class="badge">Trinity Desktop (TDE)</span>
        <span class="badge badge-green">Universal R14.1.x</span>
        <span class="badge badge-purple">x86_64</span>
      </div>
      <h1>Q4WIN10 <span class="version-pill">v${LATEST_VERSION}</span></h1>
      <p class="lead">Lightweight, modern Windows 10 &amp; 11 inspired window decoration plugin for Trinity Desktop Environment (TDE / Twin).</p>
    </header>

    <!-- Method 1: APT Repository -->
    <div class="card">
      <h2>
        <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#38bdf8" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" y1="15" x2="12" y2="3"/></svg>
        Method 1: Automatic APT Repository Installation
      </h2>
      <p style="color: var(--text-muted); font-size: 0.92rem;">
        Add the official repository to your system to receive automatic updates alongside your system packages:
      </p>

      <div class="code-container">
        <button class="copy-btn" onclick="copyCode('apt-cmd', this)">Copy</button>
        <pre id="apt-cmd">echo "deb [trusted=yes] https://seb3773.github.io/tde-win-deco-Q4WIN10/ stable main" | sudo tee /etc/apt/sources.list.d/tde-win-deco-q4win10.list
sudo apt update
sudo apt install tde-win-deco-q4win10</pre>
      </div>

      <p style="color: var(--text-muted); font-size: 0.85rem; margin-top: 10px;">
        * Supports Debian (Bullseye, Bookworm, Trixie), Ubuntu (Jammy, Noble) and Q4OS running Trinity Desktop R14.1.x.
      </p>
    </div>

    <!-- Method 2: Direct Downloads -->
    <div class="card">
      <h2>
        <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#38bdf8" stroke-width="2"><path d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-4l-4 4m0 0l-4-4m4 4V4"/></svg>
        Method 2: Direct Package Downloads
      </h2>
      <p style="color: var(--text-muted); font-size: 0.92rem;">
        Prefer offline installation? Download pre-built standalone binaries directly:
      </p>

      <div class="downloads-grid">
        <div class="download-card">
          <div class="download-header">
            <span class="download-title">Q4OS Installer (.qsi)</span>
            <span class="download-tag">Q4OS 1-Click</span>
          </div>
          <p class="download-desc">Graphical one-click installer designed specifically for Q4OS Trinity desktop environment.</p>
          <a href="${LATEST_QSI_NAME}" class="btn-download">
            Download .qsi
          </a>
        </div>

        <div class="download-card">
          <div class="download-header">
            <span class="download-title">Debian Package (.deb)</span>
            <span class="download-tag">Universal x86_64</span>
          </div>
          <p class="download-desc">Standard standalone package for Trinity Desktop / Debian-based systems.</p>
          <a href="pool/main/t/tde-win-deco-q4win10/${LATEST_DEB_NAME}" class="btn-download">
            Download .deb
          </a>
        </div>
      </div>
      <p style="color: var(--text-muted); font-size: 0.85rem; margin-top: 16px;">
        * Note: Once installed, open <strong>Trinity Control Center &rarr; Appearance &amp; Themes &rarr; Window Decorations</strong> and select <strong>Q4WIN10</strong>.
      </p>
    </div>

    <!-- Key Features -->
    <div class="card">
      <h2>
        <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#38bdf8" stroke-width="2"><path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/></svg>
        Key Capabilities &amp; Architecture
      </h2>

      <div class="features-grid">
        <div class="feature-item">
          <span class="feature-icon">🪟</span>
          <div class="feature-title">Windows 11 &amp; 10 Modes</div>
          <div class="feature-text">Floating rounded titlebar buttons, optional 8px rounded window corners with smooth clipping, and flat accent colors.</div>
        </div>

        <div class="feature-item">
          <span class="feature-icon">🌙</span>
          <div class="feature-title">Dark Mode Feature</div>
          <div class="feature-text">Toggleable high-contrast white button icons designed specifically for dark desktop themes.</div>
        </div>

        <div class="feature-item">
          <span class="feature-icon">🎨</span>
          <div class="feature-title">Seamless Menubar Matching</div>
          <div class="feature-text">Context-aware X11 Atom integration (<code>_Q4WIN10_MENUBAR_HEIGHT</code>) with static caching for zero per-frame X11 overhead.</div>
        </div>

        <div class="feature-item">
          <span class="feature-icon">⚙️</span>
          <div class="feature-title">Graphical Configuration</div>
          <div class="feature-text">Interactive control panel in Trinity Control Center with embedded branding logo and live setting changes.</div>
        </div>

        <div class="feature-item">
          <span class="feature-icon">⚡</span>
          <div class="feature-title">Maximum Optimization</div>
          <div class="feature-text">Aggressive GCC flags (<code>-O2</code>, <code>-flto</code>, <code>-fno-exceptions</code>) and <code>sstrip</code>: ~68 KB plugin, ~25 KB package.</div>
        </div>

        <div class="feature-item">
          <span class="feature-icon">🚀</span>
          <div class="feature-title">Universal Compatibility</div>
          <div class="feature-text">Decoupled from tdebase; standalone single binary fully compatible across all TDE R14.1.x versions.</div>
        </div>
      </div>
    </div>

    <!-- Screenshots -->
    <div class="card">
      <h2>
        <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#38bdf8" stroke-width="2"><rect x="3" y="3" width="18" height="18" rx="2" ry="2"/><circle cx="8.5" cy="8.5" r="1.5"/><polyline points="21 15 16 10 5 21"/></svg>
        Preview
      </h2>
      <div class="screenshots-grid">
        <div class="screenshot-thumb" onclick="openModal('screenshots/ALL.png')">
          <img src="screenshots/ALL.png" alt="Q4WIN10 Window Decoration Overview">
        </div>
      </div>
    </div>

    <!-- Footer -->
    <footer>
      <p>Source Code &amp; Releases: <a href="https://github.com/seb3773/tde-win-deco-Q4WIN10" target="_blank" rel="noopener">github.com/seb3773/tde-win-deco-Q4WIN10</a></p>
      <p style="margin-top: 6px;">Developed with ❤️ for the Trinity Desktop Environment &amp; Q4OS community.</p>
      <p class="footer-links">
        <a href="http://trinitydesktop.org/" target="_blank" rel="noopener">http://trinitydesktop.org/</a> &bull; 
        <a href="https://www.q4os.org/" target="_blank" rel="noopener">https://www.q4os.org/</a> &bull; 
        <a href="https://www.q4os.org/forum/index.php" target="_blank" rel="noopener">https://www.q4os.org/forum/index.php</a>
      </p>
    </footer>

  </div>

  <!-- Lightbox Modal -->
  <div id="imageModal" class="modal" onclick="closeModal()">
    <span class="modal-close">&times;</span>
    <img id="modalImg" src="" alt="Enlarged screenshot" onclick="event.stopPropagation()">
  </div>

  <script>
    function copyCode(id, btn) {
      const text = document.getElementById(id).innerText;
      navigator.clipboard.writeText(text).then(() => {
        const orig = btn.innerText;
        btn.innerText = "Copied!";
        setTimeout(() => btn.innerText = orig, 2000);
      });
    }

    function openModal(src) {
      document.getElementById('modalImg').src = src;
      document.getElementById('imageModal').classList.add('active');
    }

    function closeModal() {
      document.getElementById('imageModal').classList.remove('active');
    }

    document.addEventListener('keydown', (e) => {
      if (e.key === 'Escape') closeModal();
    });
  </script>
</body>
</html>
EOF

# Also save a copy of index.html in main branch repo
cp -a "$PAGES_DIR/index.html" "$REPO_DIR/index.html"

# Git commit and push to gh-pages
echo "Committing and pushing to gh-pages branch..."
(
    cd "$PAGES_DIR"
    git add -A
    git commit -m "Update APT repository and download portal: $(date +'%Y-%m-%d %H:%M:%S')" || echo "No changes to commit."
    git push origin "$PAGES_BRANCH"
)

echo "Cleaning up temporary directory..."
rm -rf "$PAGES_DIR"

echo "=================================================="
echo " SUCCESS: APT repository updated on gh-pages!"
echo " URL: https://seb3773.github.io/tde-win-deco-Q4WIN10/"
echo "=================================================="
