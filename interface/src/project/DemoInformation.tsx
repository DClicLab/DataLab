import React, { Component,useEffect, useState} from 'react';
import { Typography, Grid, Card, CardContent, createStyles, WithStyles, Theme, withStyles } from '@material-ui/core';
import { WebSocketControllerProps, SectionContent, webSocketController } from '../components';
import { LineChart, Line, CartesianGrid, XAxis, YAxis, ResponsiveContainer, Brush, Tooltip} from 'recharts'

import { ENDPOINT_ROOT, WEB_SOCKET_ROOT } from '../api';
import { toInteger } from 'lodash';
import { redirectingAuthorizedFetch } from '../authentication';
// import { Time } from '../ntp/types';


export const SENSOR_VALUE_WEBSOCKET_URL = WEB_SOCKET_ROOT + "sensorValue";

function compareSensorValues(a: SensorValue, b: SensorValue) {
  if (a.name < b.name) {
    return -1;
  }
  if (a.name > b.name) {
    return 1;
  }
  return 0;
}

export interface SensorValue {
  name: string,
  val: number,
  ts: number,
  classStyle: string,
  resetAnim: boolean,
  animSet: () => void
}


const mystyles = (theme: Theme) => createStyles(
  {
    "off": {color:"dark-grey"},
    "animation_trigger": {
      "animation-name": `$animateElement`,
      "animation-duration": "0.1s",
    },
    
    "@keyframes animateElement": {
      "0%": { "background-color": "red" },
      "100%": { "background-color": "white" },
    }
  }
  );
  
  
  
  type DemoInformationProps = WithStyles<typeof mystyles> & WebSocketControllerProps<SensorValue>;
  
  interface GraphProps{
    sensorName:string,
    graphData: any[]
  }
  interface IState {
    sens: SensorValue[],
    graphData: any[]
  }
class DemoInformation extends Component<DemoInformationProps, IState> {
  constructor(props: DemoInformationProps) {
    super(props);
    this.state = {
      sens: [],
      graphData:[]
    }
  }

  resetAnim = (() => {
    var sensorlist :SensorValue[]=[];  
    this.state.sens.forEach( (sensor: SensorValue) => {
      if (sensor.resetAnim){
        sensor.classStyle=this.props.classes.off;
      }
      sensorlist.push(sensor);
    })    
    this.setState({sens:sensorlist})
  });
  
  updateSensorList = (sensor: SensorValue) => {
    const sensors = this.state.sens!.filter(u => u.name !== sensor.name);
    sensor.classStyle = this.props.classes.animation_trigger;
    sensor.resetAnim=true;
    if (sensors.length === this.state.sens!.length) {
      this.props.enqueueSnackbar("New sensor reporting: " + sensor.name, {
        variant: 'success',
      });
    }
    sensors.push(sensor);
    let point:any={};
    point.ts = +(new Date())
    point[sensor.name]=sensor.val;

    this.setState({ sens: sensors,graphData:[...this.state.graphData, point] });
  }

  componentDidMount(){
      var d = new Date();
      d.setMilliseconds(0);
      fetch(ENDPOINT_ROOT+"settime", {
        method: 'POST',
        body: JSON.stringify({'time':d.getDay()  + "-" + (d.getMonth()+1) + "-" + d.getFullYear() + " " + d.getHours() + ":" + d.getMinutes()+ ":" + d.getSeconds()  }),
        // body: toInteger(new Date().getTime()/1000).toString(),
        headers: new Headers({
          'Content-Type': 'application/json'
        })      
      }).then( (response)=> {
        if (response.status === 200) {
          this.props.enqueueSnackbar("Time set successfully", { variant: 'success' });
        } else {
          throw Error("Error setting time, status code: " + response.status);
        }
      })
      .catch(error => {
        this.props.enqueueSnackbar(error.message || "Problem setting the time", { variant: 'error' });
      });
    }
  componentDidUpdate(prevProps: DemoInformationProps) {
    if (prevProps.data !== undefined && (prevProps.data!.val !== this.props.data?.val || prevProps.data!.name !== this.props.data!.name)) {
      // console.log("update ",prevProps.data, this.props.data)
      this.updateSensorList(this.props.data!)
    }
  }

  render() {
    return (
      <SectionContent title='DataLab - Live view' titleGutter>
        <Typography variant="body1" paragraph>
          This page displays live information returned by DataLab.
          You should see appearing your sensor and their latest data.
        </Typography>
        <SectionContent title='Live sensor values' titleGutter>
          {/* <Grid container spacing={1}> */}
        <Grid container spacing={3}>
            {
              this.state.sens!.sort(compareSensorValues).map(sensor => (
                <>

                <Grid item key={sensor.name} xs={3}>
                {/* <Card><CardContent> */}
                <SensorValueForm {...sensor}  animSet={this.resetAnim} />
                {/* </CardContent></Card> */}
                </Grid>

                <Grid item key={"G-"+sensor.name} xs={9}>
                <GraphView graphData={this.state.graphData}  sensorName={sensor.name} />
                </Grid>
                </>

              ))
            }
          </Grid>
        </SectionContent>


      </SectionContent>
    )
  }

}
export default withStyles(mystyles)(webSocketController(SENSOR_VALUE_WEBSOCKET_URL, 100, DemoInformation));

// export default DemoInformation;

function GraphView(props:GraphProps){
  var RenderTick = (tick: number) => ((new Date(tick ).toLocaleDateString("fr-FR")) + " " + (new Date(tick ).toLocaleTimeString("fr-FR")));

  return (
    <ResponsiveContainer width="100%" height={300}>
    <LineChart height={300} data={props.graphData} syncId="datalab" >
      <XAxis dataKey="ts" tickFormatter={RenderTick} />
      <YAxis />
      <Tooltip labelFormatter={RenderTick} />
      <CartesianGrid stroke="#eee" strokeDasharray="5 5" />
      <Line type="monotone" connectNulls={true} dataKey={props.sensorName} isAnimationActive={false} />
      {/* <Line type="monotone" dataKey="pv" stroke="#82ca9d" /> */}
    </LineChart>
      </ResponsiveContainer>

  )
}


class SensorValueForm extends Component<SensorValue> {

  render() {
    const { val, name, classStyle,animSet } = this.props;
    return (
        <Card>
          <CardContent>
            <Typography gutterBottom variant="h5" component="h2">
              {name}
            </Typography>
            <Typography variant="body2" component="p"  >
              Latest value:
            </Typography>
            <Typography variant="h5" className={classStyle} onAnimationEnd={animSet}>{val}</Typography>
          </CardContent>
        </Card>
    )
  }
}
