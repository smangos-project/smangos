import type {SidebarsConfig} from '@docusaurus/plugin-content-docs';

const sidebars: SidebarsConfig = {
  tutorialSidebar: [
    {
      type: 'category',
      label: 'Features',
      link: {type: 'doc', id: 'features/index'},
      items: [
        {
          type: 'category',
          label: 'Client',
          items: [
            'features/client/casc-bypass',
            'features/client/cert-bypass',
            'features/client/debug-overlay',
            'features/client/shader-utilities',
            'features/client/control-server',
          ],
        },
        'features/server',
        'features/extractors',
        'features/bots',
      ],
    },
    'setup',
    'differences',
  ],
};

export default sidebars;
