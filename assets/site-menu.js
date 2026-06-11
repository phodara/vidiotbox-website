(function () {
  const menuItems = [
    { href: 'index.html', label: 'Vidiotbox-website' },
    { href: 'robofacedocs.html', label: 'Vidiotbox-hardware Docs' },
    { href: 'https://www.instagram.com/phodara', label: 'Follow me on Instagram', external: true },
    { href: 'https://www.paulhodara.com', label: 'My Photgraphy', external: true },
    { href: 'https://www.linkedin.com/in/paulhodara/', label: 'Connect with me on LinkedIn', external: true },
    { href: 'https://github.com/phodara', label: 'My stuff on GitHub', external: true },
    { href: 'https://www.thelivingmuseum.org', label: 'The Living Museum', external: true },
    { href: 'businesscard.html', label: 'My Business Card' }
  ];

  function createMenu() {
    const siteMenu = document.createElement('nav');
    siteMenu.className = 'site-menu';
    siteMenu.dataset.open = 'false';
    siteMenu.setAttribute('aria-label', 'Site navigation');

    const menuToggle = document.createElement('button');
    menuToggle.className = 'menu-toggle';
    menuToggle.type = 'button';
    menuToggle.setAttribute('aria-label', 'Open menu');
    menuToggle.setAttribute('aria-expanded', 'false');
    menuToggle.appendChild(document.createElement('span'));

    const menuPanel = document.createElement('div');
    menuPanel.className = 'menu-panel';

    menuItems.forEach(function (item) {
      const link = document.createElement('a');
      link.href = item.href;
      link.textContent = item.label;
      if (item.external) {
        link.target = '_blank';
        link.rel = 'noopener';
      }
      menuPanel.appendChild(link);
    });

    siteMenu.appendChild(menuToggle);
    siteMenu.appendChild(menuPanel);
    return { siteMenu, menuToggle };
  }

  function mountMenu() {
    const currentScript = document.currentScript;
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
