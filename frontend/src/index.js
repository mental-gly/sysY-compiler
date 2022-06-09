import $ from 'jquery';
import * as echarts from 'echarts';

var dom = document.getElementById('root');
//用于使chart自适应高度和宽度,通过窗体高宽计算容器高宽
var resizeMainContainer = function () {
  dom.style.width = window.innerWidth+'px';
  dom.style.height = window.innerHeight*0.8+'px';
};
//设置div容器高宽
resizeMainContainer();

var myChart = echarts.init(dom, 'light', {
  renderer: 'canvas',
  useDirtyRect: false
});
var option;

myChart.showLoading();
$.getJSON('./ast.json', function (data) {
  myChart.hideLoading();
  data.children.forEach(function (datum, index) {
    index % 2 === 0 && (datum.collapsed = true);
  });
  myChart.setOption(
    (option = {
      tooltip: {
        trigger: 'item',
        triggerOn: 'mousemove'
      },
      series: [
        {
          type: 'tree',
          data: [data],
          top: '1%',
          left: '7%',
          bottom: '1%',
          right: '20%',
          symbolSize: 7,
          label: {
            position: 'left',
            verticalAlign: 'middle',
            align: 'right',
            fontSize: 9
          },
          leaves: {
            label: {
              position: 'right',
              verticalAlign: 'middle',
              align: 'left'
            }
          },
          emphasis: {
            focus: 'descendant'
          },
          expandAndCollapse: true,
          animationDuration: 550,
          animationDurationUpdate: 750
        }
      ]
    })
  );
});

if (option && typeof option === 'object') {
  myChart.setOption(option);
}

