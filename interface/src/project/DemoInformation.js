import React, { Component } from 'react';
import { ENDPOINT_ROOT } from '../constants/Env';

import { withStyles } from '@material-ui/core/styles';
import Table from '@material-ui/core/Table';
import TableHead from '@material-ui/core/TableHead';
import TableCell from '@material-ui/core/TableCell';
import TableBody from '@material-ui/core/TableBody';
import TableRow from '@material-ui/core/TableRow';
import Typography from '@material-ui/core/Typography';

import SectionContent from '../components/SectionContent';

import Chart from 'chart.js';

export const DEMO_SETTINGS_ENDPOINT = ENDPOINT_ROOT + "demoSettings";
export const DEMO_SETTINGS_ENDPOINT_VAL = ENDPOINT_ROOT + "../val";

const styles = theme => ({
  fileTable: {
    marginBottom: theme.spacing(2)
  }
});

function processSensorData(result,data){
  if (data==undefined)
    data=[];
  data.push({
    t:new Date(),
    y:result.last
  })
  return data;
}

function getData(data){
  if (data["Sensor data"]== undefined){
    data["Sensor data"]={};
    data['Sensor data'].title = 'Sensor data';
    data['Sensor data'].data=[];
  }    
  fetch(DEMO_SETTINGS_ENDPOINT_VAL)
      .then(res => res.json())
      .then(
        (result) => {
          processSensorData(result,data["Sensor data"].data);
        },
        // Note: it's important to handle errors here
        // instead of a catch() block so that we don't swallow
        // exceptions from actual bugs in components.
        (error) => {
          // this.setState({
          //   isLoaded: true,
          //   error
          // });
        });
  return data;
}


class DemoInformation extends Component {
  constructor(props) {
    super(props);
    
    let ldata=[];
    this.state = {
      data: getData(ldata)
    };
  }

  componentDidMount() {
//    this.props.loadData();
    window.setInterval(() => {
      this.setState({
        data: getData(this.state.data)
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
          <SensorChart
            data={this.state.data['Sensor data'].data}
            title={this.state.data['Sensor data'].title}
            color="rgba(255, 99, 132, 0.5)"
          />
        </div>
              </TableCell>
            </TableRow>
            <TableRow>
              <TableCell>
              <div className="main chart-wrapper">
          {/* <LineChart
            data={this.state.data[1].data}
            title={this.state.data[1].title}
            color="#3E517A"
          /> */}
        </div>
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

class SensorChart extends React.Component {
  constructor(props) {
    super(props);
    this.canvasRef = React.createRef();
  }

  componentDidUpdate() {
    //this.myChart.data.labels = this.props.data.map(d => d.time);
    //this.myChart.data.labels = this.props.data.map(d => d.label);
    //this.myChart.data.datasets[0].data = this.props.data.map(d => d.time, d.value);
    this.myChart.data.datasets[0].data = this.props.data;
    this.myChart.update();
  }

  componentDidMount() {
    
    this.myChart = new Chart(this.canvasRef.current, {
      type: 'line',
      options: {
        maintainAspectRatio: false,
        responsive: true,
        scales: {
          xAxes: [{
						type: 'time',
						display: true,
						scaleLabel: {
							display: true,
							labelString: 'Date'
						},
						ticks: {
							major: {
								fontStyle: 'bold',
								fontColor: '#FF0000'
							}
						}
					}],
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
       // labels: this.props.data.map(d => d.label),
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
    return <canvas ref={this.canvasRef} /> ;
  }
  
}





export default withStyles(styles)(DemoInformation);
