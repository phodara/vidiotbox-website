(function () {
  const menuCss = `
    .site-menu .menu-toggle {
      width: 116px;
      height: 44px;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      padding: 0 10px;
      line-height: 1;
    }

    .site-menu .menu-toggle::before,
    .site-menu .menu-toggle::after {
      content: none !important;
      display: none !important;
    }

    .site-menu .menu-toggle-label {
      color: currentColor;
      display: block;
      width: auto;
      height: auto;
      background: transparent !important;
      font: 800 13px/1 Arial, Helvetica, sans-serif;
      letter-spacing: 1px;
      white-space: nowrap;
    }

    .site-menu .menu-toggle-puppet {
      position: relative;
      width: 22px;
      height: 30px;
      flex: 0 0 22px;
      color: currentColor;
      background: transparent !important;
    }

    .site-menu .menu-toggle-puppet span {
      position: absolute;
      display: block;
      background: currentColor;
      opacity: 1;
      transition: transform .2s ease;
    }

    .site-menu .menu-toggle-puppet .head {
      top: 1px;
      left: 8px;
      width: 6px;
      height: 6px;
      border-radius: 50%;
    }

    .site-menu .menu-toggle-puppet .body {
      top: 8px;
      left: 10px;
      width: 2px;
      height: 12px;
    }

    .site-menu .menu-toggle-puppet .arm,
    .site-menu .menu-toggle-puppet .leg {
      width: 2px;
      transform-origin: 1px 1px;
    }

    .site-menu .menu-toggle-puppet .arm {
      top: 10px;
      height: 10px;
    }

    .site-menu .menu-toggle-puppet .arm-left {
      left: 10px;
      transform: rotate(65deg);
    }

    .site-menu .menu-toggle-puppet .arm-right {
      left: 10px;
      transform: rotate(-65deg);
    }

    .site-menu .menu-toggle-puppet .leg {
      top: 19px;
      height: 11px;
    }

    .site-menu .menu-toggle-puppet .leg-left {
      left: 10px;
      transform: rotate(18deg);
    }

    .site-menu .menu-toggle-puppet .leg-right {
      left: 10px;
      transform: rotate(-18deg);
    }

    .site-menu[data-open="true"] .menu-toggle-puppet .arm-left {
      transform: rotate(132deg);
    }

    .site-menu[data-open="true"] .menu-toggle-puppet .arm-right {
      transform: rotate(-132deg);
    }

    .site-menu[data-open="true"] .menu-toggle-puppet .leg-left {
      transform: rotate(52deg);
    }

    .site-menu[data-open="true"] .menu-toggle-puppet .leg-right {
      transform: rotate(-52deg);
    }

    .site-menu[data-open="true"] .menu-toggle span {
      opacity: 1;
    }
  `;

  const menuItems = [
    { href: 'index.html', label: 'VIDIOTBOX-WEBSITE (HOME)' },
    { href: 'vidiotbox-hardware_docs.html', label: 'Vidiotbox-hardware Docs' },
    { href: 'lilygo-t-display-c5.html', label: 'POCKET PROWLER WIFI/BLE ANALYZER' },
    { href: 'pictograms.html', label: 'Pictograms' },
    { href: 'https://main.vidiotbox.pages.dev/Clownsmall', label: 'Facimator', external: true },
    { href: 'https://www.instagram.com/phodara', label: 'Follow me on Instagram', external: true },
    { href: 'https://www.paulhodara.com', label: 'My Photography', external: true },
    { href: 'https://www.linkedin.com/in/paulhodara/', label: 'Connect with me on LinkedIn', external: true },
    { href: 'https://github.com/phodara', label: 'My stuff on GitHub', external: true },
    { href: 'https://www.thelivingmuseum.org', label: 'The Living Museum', external: true },
    { href: 'businesscard.html', label: 'My Business Card' }
  ];

  function injectMenuStyles() {
    if (document.getElementById('shared-menu-project-styles')) return;

    const style = document.createElement('style');
    style.id = 'shared-menu-project-styles';
    style.textContent = `
      .menu-panel {
        width: min(260px, calc(100vw - 28px));
        max-height: calc(100vh - 82px);
        overflow-y: auto;
      }
      .menu-section {
        margin: 4px 2px 0;
        color: #b7bbc6;
        font-size: 10px;
        font-weight: 800;
        letter-spacing: 1.4px;
        text-transform: uppercase;
      }
    `;
    document.head.appendChild(style);
  }

  function applyExternalAttrs(link) {
    link.target = '_blank';
    link.rel = 'noopener';
  }

  function createPuppetToggleContent() {
    const puppet = document.createElement('span');
    puppet.className = 'menu-toggle-puppet';
    puppet.setAttribute('aria-hidden', 'true');

    ['head', 'body', 'arm arm-left', 'arm arm-right', 'leg leg-left', 'leg leg-right'].forEach(function (className) {
      const part = document.createElement('span');
      part.className = className;
      puppet.appendChild(part);
    });

    const label = document.createElement('span');
    label.className = 'menu-toggle-label';
    label.textContent = 'MY TOYS';

    return [puppet, label];
  }

  function injectMenuCss() {
    if (document.getElementById('site-menu-puppet-css')) return;
    const style = document.createElement('style');
    style.id = 'site-menu-puppet-css';
    style.textContent = menuCss;
    document.head.appendChild(style);
  }

  function createMenu() {
    const siteMenu = document.createElement('nav');
    const opensByDefault = false;
    siteMenu.className = 'site-menu';
    siteMenu.dataset.open = String(opensByDefault);
    siteMenu.setAttribute('aria-label', 'Site navigation');

    const menuToggle = document.createElement('button');
    menuToggle.className = 'menu-toggle';
    menuToggle.type = 'button';
    menuToggle.setAttribute('aria-label', opensByDefault ? 'Close menu' : 'Open menu');
    menuToggle.setAttribute('aria-expanded', String(opensByDefault));
    createPuppetToggleContent().forEach(function (child) {
      menuToggle.appendChild(child);
    });

    const menuPanel = document.createElement('div');
    menuPanel.className = 'menu-panel';

    menuItems.forEach(function (item) {
      if (item.type === 'section') {
        const heading = document.createElement('div');
        heading.className = 'menu-section';
        heading.textContent = item.label;
        menuPanel.appendChild(heading);
        return;
      }

      const link = document.createElement('a');
      link.href = item.href;
      link.textContent = item.label;
      if (item.external) {
        applyExternalAttrs(link);
      }
      menuPanel.appendChild(link);
    });

    siteMenu.appendChild(menuToggle);
    siteMenu.appendChild(menuPanel);
    return { siteMenu, menuToggle };
  }

  function mountMenu() {
    injectMenuStyles();

    const currentScript = document.currentScript;
    injectMenuCss();
    const menu = createMenu();
    const siteMenu = menu.siteMenu;
    const menuToggle = menu.menuToggle;

    function setMenuOpen(isOpen) {
      siteMenu.dataset.open = String(isOpen);
      menuToggle.setAttribute('aria-expanded', String(isOpen));
      menuToggle.setAttribute('aria-label', isOpen ? 'Close menu' : 'Open menu');
    }

    menuToggle.addEventListener('click', function () {
      setMenuOpen(siteMenu.dataset.open !== 'true');
    });

    document.addEventListener('click', function (event) {
      if (!siteMenu.contains(event.target)) setMenuOpen(false);
    });

    document.addEventListener('keydown', function (event) {
      if (event.key === 'Escape') setMenuOpen(false);
    });

    siteMenu.querySelectorAll('.menu-panel a').forEach(function (link) {
      link.addEventListener('click', function () {
        setMenuOpen(false);
      });
    });

    if (currentScript && currentScript.parentNode) {
      currentScript.parentNode.insertBefore(siteMenu, currentScript);
    } else {
      document.body.insertBefore(siteMenu, document.body.firstChild);
    }
  }

  mountMenu();
}());
