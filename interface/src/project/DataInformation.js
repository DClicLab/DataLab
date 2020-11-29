import React, { Component } from 'react';
import { ENDPOINT_ROOT } from '../constants/Env';

import { withStyles } from '@material-ui/core/styles';
import Table from '@material-ui/core/Table';
import TableHead from '@material-ui/core/TableHead';
import TableCell from '@material-ui/core/TableCell';
import TableBody from '@material-ui/core/TableBody';
import TableRow from '@material-ui/core/TableRow';
import Typography from '@material-ui/core/Typography';
import { withSnackbar } from 'notistack';


import SectionContent from '../components/SectionContent';

import Chart from 'chart.js';

export const DATA_SETTINGS_ENDPOINT = ENDPOINT_ROOT + "dataSettings";
export const DATA_SETTINGS_ENDPOINT_VAL = ENDPOINT_ROOT + "../val";

const styles = theme => ({
  fileTable: {
    marginBottom: theme.spacing(2)
  },
  chart:{
    position:"relative", 
    height:"40vh",
    width:"60vw"
  }
});

const chartColors = {
  red: 'rgb(255, 99, 132)',
  orange: 'rgb(255, 159, 64)',
  yellow: 'rgb(255, 205, 86)',
  green: 'rgb(75, 192, 192)',
  blue: 'rgb(54, 162, 235)',
  purple: 'rgb(153, 102, 255)',
  grey: 'rgb(231,233,237)'
};

function processHistoricalData(csvstr){

  // csvstr.split("\n").map(function(row){
  //   if (a!=""){
  //   a=row.split(",");
  //   dataList[a[0]].push({t:new Date(a[1] * 1000),y:a[2]}) }})
  
  // ;

}

function processSensorData(result,dataList){
  if (dataList===undefined)
    dataList={};
  for (const label in result) {
    if (result.hasOwnProperty(label)) {
      if (dataList[label]===undefined)
        dataList[label]=[]
        dataList[label].push({
          t:new Date(),
          y:result[label]
        })
    }
  }
  return dataList;
}

function getData(dataList){
  fetch(DATA_SETTINGS_ENDPOINT_VAL)
      .then(res => res.json())
      .then(
        (result) => {
          processSensorData(result,dataList);
        },
        (error) => {
          // this.setState({
          //   isLoaded: true,
          //   error
          // });
        });
  return dataList;
}


class DataInformation extends Component {
  constructor(props) {
    super(props);
    
    let dataList=[];
    this.state = {
      dataList: getData(dataList)
    };
  }

  componentDidMount() {
//    this.props.loadData();
  const { enqueueSnackbar } = this.props;
    window.setInterval(() => {
      this.setState({
        dataList: getData(this.state.dataList)
      })
    }, 2000)
    var d = new Date();
    fetch("/settime", {
      method: 'POST',
      body: JSON.stringify({ 'time':d.getDate()  + "-" + (d.getMonth()+1) + "-" + d.getFullYear() + " " + d.getHours() + ":" + d.getMinutes()  }),
      headers: new Headers({
        'Content-Type': 'application/json'
      })
      
    }).then( ()=> {enqueueSnackbar("Time updated successfully.", { variant: 'success' });})
    .catch(error => {
      const errorMessage = error.message || "Unknown error";
      enqueueSnackbar("Problem setting time: " + errorMessage, {
        variant: 'error',
      })});
  }
  
  render() {
    const { classes,enqueueSnackbar } = this.props;



    return (
      <SectionContent title="DataLab v2" titleGutter>
        <Typography variant="body1" paragraph>
          This screen monitors the current values from your sensors.
        </Typography>
        <Table className={classes.fileTable}>
          <TableHead>
            <TableRow>
              <TableCell>
                Sensor value
              </TableCell>
              <TableCell>
                
              </TableCell>
            </TableRow>
          </TableHead>
          <TableBody>
      
          {Object.entries(this.state.dataList).map((data,index) => (
          <TableRow key={index} className={classes.row}>
              <TableCell>
              <h3>{data[0]}</h3></TableCell>
              <TableCell>
              <div className={classes.chart}>
                        <SensorChart2
                        dataList={data[1]}
                        label={data[0]}
                        index={index}
                      />
              </div>
              </TableCell>
              </TableRow>
          ))}
          </TableBody>
        </Table>
        <Typography variant="body1" paragraph>
          See the project <a href="https://github.com/rjwats/esp8266-react/">README</a> for a full description of the data project.
        </Typography>
      </SectionContent>
    )
  }

}

class SensorChart2 extends React.Component {
  constructor(props) {
    super(props);
    this.canvasRef = React.createRef();
    console.log("creating a SensorChart2");
    console.log(this.props);
    
  }

  componentDidUpdate() {
        const datas = this.props.dataList;
        if (this.myChart.data.datasets.length===0)
          this.myChart.data.datasets.push(
            {
              "label":this.props.label,
              data:datas,
              backgroundColor:chartColors[Object.keys(chartColors)[this.props.index]],
              borderColor:chartColors[Object.keys(chartColors)[this.props.index]],
              "lineTension":0.1
          })          
        else{
          this.myChart.data.datasets[0].data=datas;
        }
    this.myChart.update();
  }

  componentDidMount() {
    
    this.myChart = new Chart(this.canvasRef.current, {
      type: 'line',
      options: {
        datasets:{//default for all
          line:{
            fill: false,
            pointRadius: 2,
            borderWidth: 1,
            lineTension: 0
          }
        },
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
              // ticks: {
              //   min: 0
              // }
            }
          ]
        }
      },
      data: {
      }
    });
  }

  render() {
    return <canvas ref={this.canvasRef} /> ;
  }
  
}

export default withSnackbar(withStyles(styles)(DataInformation));