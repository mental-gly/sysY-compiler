const data = [{
    name: '阿巴阿巴',
    label: {
      backgroundColor: 'rgba(168, 0, 252, 1)',
      borderRadius: [22, 11, 11, 6],
      padding: [16, 16, 16, 16],
    },
    children: [
      {
        name: '产品1',
        idss: 1,
        collapsed: false,
        label: {
          backgroundColor: 'rgba(252, 25, 0, 1)',
          borderRadius: [22, 11, 11, 6],
          padding: [16, 16, 16, 16],
        },
        children: [
          {
            name: '基线1',
            children: [
              { name: '基线1', value: 3938, collapsed: true, },
              { name: '基线2', value: 3812, collapsed: true, },
              { name: '基线3', value: 6714, collapsed: true,  },
              { name: '基线4', value: 743, collapsed: true,  },
            ],
          },
          {
            name: '基线2',
            children: [
              { name: '基线1', value: 3534, collapsed: true,  },
              { name: '基线2', value: 5731, collapsed: true,  },
              { name: '基线3', value: 7840, collapsed: true,  },
              { name: '基线4', value: 5914, collapsed: true,  },
              { name: '基线5', value: 3416, collapsed: true,  },
            ],
          },
          {
            name: '基线3',
            children: [{ name: 'AspectRatioBanker', value: 7074 }],
          },
        ],
      },
      {
        name: '产品2',
        idss: 2,
        collapsed: true,
        label: {
          backgroundColor: 'rgba(242, 144, 7, 1)',
          borderRadius: [22, 11, 11, 6],
          padding: [16, 16, 16, 16],
        },
        children: [
          { name: '基线1', collapsed: true, },
          { name: '基线1', value: 1759, collapsed: true, },
          { name: '基线2', value: 2165, collapsed: true, },
          { name: '基线3', value: 586, collapsed: true, },
          { name: '基线4', value: 3331, collapsed: true, },
          { name: '基线5', value: 772, collapsed: true, },
          { name: '基线6', value: 3322, collapsed: true, },
        ],
      },
      {
        name: '产品3',
        idss: 3,
        collapsed: true,
        label: {
          backgroundColor: 'rgba(252, 248, 0, 1)',
          borderRadius: [22, 11, 11, 6],
          padding: [16, 16, 16, 16],
        },
        children: [
          { name: '基线1', value: 8833, collapsed: true, },
          { name: '基线2', value: 1732, collapsed: true, },
          { name: '基线3', value: 3623, collapsed: true, },
          { name: '基线4', value: 10066, collapsed: true, },
        ],
      },
    ],
  }];
  
  export default data;