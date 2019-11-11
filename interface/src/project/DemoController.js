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
  componentDidMount() {
    this.props.loadData();
  }

  componentDidUpdate(){
    console.log("got an update",this.props.data);
  }
  render() {
    const { data, fetched, errorMessage, saveData, loadData, handleSliderChange, handleSensorChange } = this.props;
    return (
      <SectionContent title="Controller" titleGutter>
        <LoadingNotification
          onReset={loadData}
          fetched={fetched}
          errorMessage={errorMessage}
          render={() =>
            <div>
            {/* <DemoControllerForm
              demoSettings={data}
              onReset={loadData}
              onSubmit={saveData}
              handleSliderChange={handleSliderChange}
            /> */}
            <SensorsControllerForm
              demoSettings={data}
              onReset={loadData}
              onSubmit={saveData}
              handleSensorChange={handleSensorChange}
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

function SensorsControllerForm(props) {
  const { demoSettings, onSubmit, onReset, handleSensorChange } = props;
  const classes = useStyles();

  console.log("We have ",demoSettings, handleSensorChange);
  return (
    <>

     <ValidatorForm onSubmit={onSubmit}>
        {demoSettings["sensors"].map((sensor,index) => (
    <div     key = {index}>
      <TextValidator 
      validators={['required']}
      errorMessages={['Server is required']}
      name={"sensorMin"+index}
      label="Sensor max"
      id={"sensorMax"+index}
      className={classes.textField}
      value={demoSettings["sensors"][index]["max"] || ''}//Why could this be undefined???
      onChange={ handleSensorChange(index,"max")}
      margin="normal"
      />
      <TextValidator 
      validators={['required']}
      errorMessages={['Server is required']}
      name={"sensorMin"+index}
      label="Sensor min"
      id={"sensorMin"+index}
      className={classes.textField}
      value={demoSettings["sensors"][index]["min"] || ''}//Why could this be undefined???
      onChange={ handleSensorChange(index,"min")}
      margin="normal"
      />
    <TextValidator 

    validators={['required']}
    errorMessages={['Server is required']}
    name={"sensorName"+index}
    label="Sensor"
    id={"sensorName"+index}
    className={classes.textField}
    value={demoSettings["sensors"][index].name || ''}//Why could this be undefined???
    onChange={ handleSensorChange(index,"name")}
    margin="normal"
    />
    
    </div>
        ))}
        <Button variant="contained" color="primary" className={classes.button} type="submit">
         Save
        </Button>
        <Button variant="contained" color="secondary" className={classes.button} onClick={onReset}>
        Reset
      </Button>
      </ValidatorForm>
      </>
   );
}

export default restComponent(DEMO_SETTINGS_ENDPOINT, DemoController);
