import React, { Component} from 'react';
import { Typography, Grid, Card, CardContent, createStyles, WithStyles, Theme, withStyles } from '@material-ui/core';
import { WebSocketControllerProps, SectionContent, webSocketController } from '../components';

import { WEB_SOCKET_ROOT } from '../api';
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

export interface IState {
  sens: SensorValue[],
}

const mystyles = (theme: Theme) => createStyles(
  {
    "off": {color:"red"},
    "animation_trigger": {
      "animation-name": `$animateElement`,
      "animation-duration": "0.1s",
    },

    "@keyframes animateElement": {
      "0%": { "background-color": "red" },
      "25%": { "background-color": "yellow" },
      "50%": { "background-color": "blue" },
      "100%": { "background-color": "green" },
    }
  }
);



type DemoInformationProps = WithStyles<typeof mystyles> & WebSocketControllerProps<SensorValue>;

class DemoInformation extends Component<DemoInformationProps, IState> {
  constructor(props: DemoInformationProps) {
    super(props);
    this.state = {
      sens: []
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
    this.setState({ sens: sensors });
  }

  componentDidUpdate(prevProps: DemoInformationProps) {
    if (prevProps.data !== undefined && (prevProps.data!.val !== this.props.data?.val || prevProps.data!.name !== this.props.data!.name)) {
      // console.log("update ",prevProps.data, this.props.data)
      this.updateSensorList(this.props.data!)
    }
  }

  render() {
    return (
      <SectionContent title='Demo Information' titleGutter>
        <Typography variant="body1" paragraph>
          This simple demo project allows you to control the built-in LED.
          It demonstrates how the esp8266-react framework may be extended for your own IoT project.
        </Typography>
        <Typography variant="body1" paragraph>
          It is recommended that you keep your project interface code under the project directory.
          This serves to isolate your project code from the from the rest of the user interface which should
          simplify merges should you wish to update your project with future framework changes.
        </Typography>
        <SectionContent title='Live sensor values' titleGutter>
          <Grid container spacing={1}>
            {
              this.state.sens!.sort(compareSensorValues).map(sensor => (
                <SensorValueForm {...sensor} key={sensor.name} animSet={this.resetAnim} />
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


class SensorValueForm extends Component<SensorValue> {

  render() {
    const { val, name, classStyle,animSet } = this.props;
    return (
      <Grid item xs={12} sm={4}>
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
      </Grid>
    )
  }
}
