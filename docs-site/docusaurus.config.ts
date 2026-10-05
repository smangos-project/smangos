import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

const config: Config = {
  title: 'smangos',
  tagline: 'A vmangos-based server for the 1.15 Classic Era client',
  favicon: 'img/favicon.ico',

  future: {
    v4: true,
  },

  url: 'https://smangos-project.github.io',
  baseUrl: '/smangos/',

  organizationName: 'smangos-project',
  projectName: 'smangos',

  onBrokenLinks: 'throw',

  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },

  presets: [
    [
      'classic',
      {
        docs: {
          sidebarPath: './sidebars.ts',
          editUrl: 'https://github.com/smangos-project/smangos/tree/development/docs-site/',
        },
        blog: false,
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],

  themeConfig: {
    image: 'img/docusaurus-social-card.jpg',
    colorMode: {
      respectPrefersColorScheme: true,
    },
    navbar: {
      title: 'smangos',
      logo: {
        alt: 'smangos logo',
        src: 'img/logo.svg',
      },
      items: [
        {
          type: 'docSidebar',
          sidebarId: 'tutorialSidebar',
          position: 'left',
          label: 'Docs',
        },
        {
          href: 'https://github.com/smangos-project/smangos',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Docs',
          items: [
            {
              label: 'Features',
              to: '/docs/features/',
            },
            {
              label: 'Setup',
              to: '/docs/setup',
            },
            {
              label: 'Differences',
              to: '/docs/differences',
            },
          ],
        },
        {
          title: 'Project',
          items: [
            {
              label: 'GitHub',
              href: 'https://github.com/smangos-project/smangos',
            },
          ],
        },
      ],
      copyright: `Copyright (c) ${new Date().getFullYear()} smangos contributors. Built with Docusaurus.`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
