import React, { Component } from 'react';

import { withStyles } from '@material-ui/core/styles';
import Table from '@material-ui/core/Table';
import TableHead from '@material-ui/core/TableHead';
import TableCell from '@material-ui/core/TableCell';
import TableBody from '@material-ui/core/TableBody';
import TableRow from '@material-ui/core/TableRow';
import Typography from '@material-ui/core/Typography';

import SectionContent from '../components/SectionContent';

import Chart from 'chart.js';


const styles = theme => ({
  fileTable: {
    marginBottom: theme.spacing(2)
  }
});

// Data generation
function getRandomArray(numItems) {
  // Create random array of objects
  let names = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
  let data = [];
  for(var i = 0; i < numItems; i++) {
    data.push({
      label: names[i],
      value: Math.round(20 + 80 * Math.random())
    });
  }
  return data;
}
function getRandomDateArray(numItems) {
  // Create random array of objects (with date)
  let data = [];
  let baseTime = new Date('2018-05-01T00:00:00').getTime();
  let dayMs = 24 * 60 * 60 * 1000;
  for(var i = 0; i < numItems; i++) {
    data.push({
      time: new Date(baseTime + i * dayMs),
      value: Math.round(20 + 80 * Math.random())
    });
  }
  return data;
}


function getData() {
  let data = [];

  data.push({
    title: 'Visits',
    data: getRandomDateArray(150)
  });

  data.push({
    title: 'Categories',
    data: getRandomArray(20)
  });

  data.push({
    title: 'Categories',
    data: getRandomArray(10)
  });

  data.push({
    title: 'Data 4',
    data: getRandomArray(6)
  });

  return data;
}

// LineChart
class LineChart extends React.Component {
  constructor(props) {
    super(props);
    this.canvasRef = React.createRef();
  }

  componentDidUpdate() {
    this.myChart.data.labels = this.props.data.map(d => d.time);
    this.myChart.data.datasets[0].data = this.props.data.map(d => d.value);
    this.myChart.update();
  }

  componentDidMount() {
    this.myChart = new Chart(this.canvasRef.current, {
      type: 'line',
      options: {
			  maintainAspectRatio: false,
        scales: {
          xAxes: [
            {
              type: 'time',
              time: {
                unit: 'week'
              }
            }
          ],
          yAxes: [
            {
              ticks: {
                min: 0
              }
            }
          ]
        }
      },
      data: {
        labels: this.props.data.map(d => d.time),
        datasets: [{
          label: this.props.title,
          data: this.props.data.map(d => d.value),
          fill: 'none',
          backgroundColor: this.props.color,
          pointRadius: 2,
          borderColor: this.props.color,
          borderWidth: 1,
          lineTension: 0
        }]
      }
    });
  }

  render() {
    return <canvas ref={this.canvasRef} />;
  }
}



class DemoInformation extends Component {
  constructor(props) {
    super(props);

    this.state = {
      data: getData()
    };
  }

  componentDidMount() {
    window.setInterval(() => {
      this.setState({
        data: getData()
      })
    }, 5000)
  }
  render() {
    const { classes } = this.props;
    return (
      <SectionContent title="Captobox 2.0" titleGutter>
        <Typography variant="body1" paragraph>
          This simple demo project allows you to control the blink speed of the built-in LED. 
          It demonstrates how the esp8266-react framework may be extended for your own IoT project.
        </Typography>
        <Typography variant="body1" paragraph>
          It is recommended that you keep your project interface code under the 'project' directory.
          This serves to isolate your project code from the from the rest of the user interface which should
          simplify merges should you wish to update your project with future framework changes.
        </Typography>
        <Typography variant="body1" paragraph>
          The demo project interface code stored in the interface/project directory:
        </Typography>
        <Table className={classes.fileTable}>
          <TableHead>
            <TableRow>
              <TableCell>
                File
              </TableCell>
              <TableCell>
                Description
              </TableCell>
            </TableRow>
          </TableHead>
          <TableBody>
          <TableRow>
              <TableCell>
                Sensor1
              </TableCell>
              <TableCell>
              <div className="main chart-wrapper">
          <LineChart
            data={this.state.data[0].data}
            title={this.state.data[0].title}
            color="#3E517A"
          />
        </div>
              </TableCell>
            </TableRow>
            <TableRow>
              <TableCell>
                ProjectMenu.js
              </TableCell>
              <TableCell>
                You can add your project's screens to the side bar here.
              </TableCell>
            </TableRow>
            <TableRow>
              <TableCell>
                ProjectRouting.js
              </TableCell>
              <TableCell>
                The routing which controls the screens of your project.
              </TableCell>
            </TableRow>
            <TableRow>
              <TableCell>
                DemoProject.js
              </TableCell>
              <TableCell>
                This screen, with tabs and tab routing.
              </TableCell>
            </TableRow>
            <TableRow>
              <TableCell>
                DemoInformation.js
              </TableCell>
              <TableCell>
                The demo information tab.
              </TableCell>
            </TableRow>
            <TableRow>
              <TableCell>
                DemoController.js
              </TableCell>
              <TableCell>
                The demo controller tab, to control the built-in LED.
              </TableCell>
            </TableRow>                    
          </TableBody>
        </Table>
        <Typography variant="body1" paragraph>
          See the project <a href="https://github.com/rjwats/esp8266-react/">README</a> for a full description of the demo project.
        </Typography>
      </SectionContent>
    )
  }

}

export default withStyles(styles)(DemoInformation);
