import EChartsReact from 'echarts-for-react';
import './App.css';
import data from './data';

const App = () => {
  // 配置
  const getOption = {
    // 标题
    title: {
      text: 'AST(抽象语法树)',
    },
    // 提示，滑过时展示数据
    tooltip: {
      trigger: 'item',
      formatter: (params) => {
        return (
          [params.name] +
          '<br/>基线数量 : ' +
          [params.data.idss] +
          '<br/>资金(万元) : ' +
          [params.data.collapsed]
        );
      },
    },
    // 主要配置
    series: [
      {
      	// 类型
        type: 'tree',
        // 数据源
        data: data,

        top: '1%',
        left: '7%',
        bottom: '1%',
        right: '20%',

        symbol: 'none', // symbolSize: 100,
		// 字体节点样式
        label: {
          backgroundColor: 'rgba(241, 191, 14, 1)',
          borderRadius: [22, 11, 11, 6],
          padding: [16, 16, 16, 16],
        },
		// 线条样式
        lineStyle: {
          color: 'rgba(221, 212, 212, 1)',
          curveness: 0.8,
          width: '0.5',
        },

        leaves: {},

        emphasis: {
          focus: 'descendant',
        },
		// 默认展开计几层
        initialTreeDepth: 1,

        expandAndCollapse: true,
        animationDuration: 550,
        animationDurationUpdate: 750,
      },
    ],
  };

  return (
    <div>
      <EChartsReact
        style={{ height: 700 }}
        option={getOption}
        notMerge={true}
        lazyUpdate={true}
        // 绑定事件
        onEvents={onclick}
      ></EChartsReact>
    </div>
  );
}

export default App;
