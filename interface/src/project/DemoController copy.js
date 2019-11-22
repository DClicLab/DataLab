import React, { Component } from 'react';
import { ValidatorForm,TextValidator } from 'react-material-ui-form-validator';

import { ENDPOINT_ROOT } from '../constants/Env';
import SectionContent from '../components/SectionContent';
import { restComponent } from '../components/RestComponent';
import LoadingNotification from '../components/LoadingNotification';

import Button from '@material-ui/core/Button';
import Typography from '@material-ui/core/Typography';
import Slider from '@material-ui/core/Slider';
import { makeStyles } from '@material-ui/core/styles';
export const DEMO_SETTINGS_ENDPOINT = ENDPOINT_ROOT + "demoSettings";

const valueToPercentage = (value) => `${Math.round(value / 255 * 100)}%`;

class DemoController extends Component {
  constructor(props) {
    super(props);
    this.state = {};
    this.state.sensors =[
      {
        "name":"my first sensor",
        "type":"temp"
      },
      {
        "name":"my sec sensored",
        "type":"hum"
      }
    ]
    console.log("restart");
    this.handleChange = this.handleChange.bind(this);

  }
  componentDidMount() {
    this.props.loadData();
  }
  handleChange(event) {
    console.log(event.target);
    this.setState({value: event.target.value});
  }
  render() {
    const { data, fetched, errorMessage, saveData, loadData, handleSliderChange , handleChange } = this.props;

    return (
      <SectionContent title="Controller" titleGutter>
        <LoadingNotification
          onReset={loadData}
          fetched={fetched}
          errorMessage={errorMessage}
          render={() =>
            <div>
            <DemoControllerForm
              demoSettings={data}
              onReset={loadData}
              onSubmit={saveData}
              handleSliderChange={handleSliderChange}
            />
            <SensorControllerForm
              demoSettings={data}
              onReset={loadData}
              onSubmit={saveData}
              handleValueChange={handleChange}
              sensors = {this.state.sensors}
            />
            </div>
          }
        />
      </SectionContent>
    )
  }
}

const useStyles = makeStyles(theme => ({
  button: {
    marginRight: theme.spacing(2),
    marginTop: theme.spacing(2),
  },
  blinkSpeedLabel: {
    marginBottom: theme.spacing(5),
  }
}));

function DemoControllerForm(props) {
  const { demoSettings, onSubmit, onReset, handleSliderChange } = props;
  const classes = useStyles();
  return (
    <ValidatorForm onSubmit={onSubmit}>
      <Typography id="blink-speed-slider" className={classes.blinkSpeedLabel}>
        Blink Speed
      </Typography>
      <Slider
        value={demoSettings.blink_speed}
        valueLabelFormat={valueToPercentage}
        aria-labelledby="blink-speed-slider"
        valueLabelDisplay="on"
        min={0}
        max={255}
        onChange={handleSliderChange('blink_speed')}
      />
      <Button variant="contained" color="primary" className={classes.button} type="submit">
        Save
      </Button>
      <Button variant="contained" color="secondary" className={classes.button} onClick={onReset}>
        Reset
      </Button>
    </ValidatorForm>
  );
}


function SensorControllerForm(props) {
  const { demoSettings, onSubmit, onReset, handleValueChange,sensors } = props;
  const classes = useStyles();
  console.log(demoSettings);
  // //state.sensors = sensors;
  // function handleSensorChange(sensor,index, event){
  //   console.log("HSE: ",event,  sensor , index);
  //   console.log(event.target.value);
  //   sensors[index].name = event.target.value;
    
  // }

  // const sensor
  // <ValidatorForm onSubmit={onSubmit}>
  //   <Typography id="blink-speed-slider" className={classes.blinkSpeedLabel}>
  //     Blink Speed
  //   </Typography>
  //   <label>
  //       Name:
  //       <input type="text" value={demoSettings.sensor[i].name} onChange={handleSliderChange.bind('value',i)} />
  //     </label>
  //   <Button variant="contained" color="primary" className={classes.button} type="submit">
  //     Save
  //   </Button>
  //   <Button variant="contained" color="secondary" className={classes.button} onClick={onReset}>
  //     Reset
  //   </Button>
  // </ValidatorForm>
  //   )
  // }

  return (
      <>
<ValidatorForm onSubmit={onSubmit}>
    {sensors.map((sensor,index) => (

<TextValidator 
key = {index}
validators={['required']}
errorMessages={['Server is required']}
name="sensor"
label="Sensor"
id={"sensor"+index}
className={classes.textField}
value={sensors[index].name}
onChange={(e) => handleValueChange}
margin="normal"
/>
    ))}
        <Button variant="contained" color="primary" className={classes.button} type="submit">
          Save
        </Button>
    </ValidatorForm>
  </>

  );
}

    

export default restComponent(DEMO_SETTINGS_ENDPOINT, DemoController);
