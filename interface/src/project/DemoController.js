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
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/MenuItem';
import FormHelperText from '@material-ui/core/FormHelperText';
import FormControl from '@material-ui/core/FormControl';
import Select from '@material-ui/core/Select';
import { array } from 'prop-types';


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
    const { data, fetched, errorMessage, saveData, loadData, handleNewSensor, handleSensorChange, handleRemoveSensor } = this.props;
    
    return (
      <SectionContent title="Controller" titleGutter>
     <Button color="primary" variant="contained" onClick={handleNewSensor}>
        Add a sensor
      </Button>

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
              handleRemoveSensor={handleRemoveSensor}
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
    marginRight: theme.spacing(1),
    marginTop: theme.spacing(2),
  },
  blinkSpeedLabel: {
    marginBottom: theme.spacing(5),
  }
}));


function SensorsControllerForm(props) {
  const { demoSettings, onSubmit, onReset, handleSensorChange,handleRemoveSensor } = props;
  const classes = useStyles();

  console.log("We have ",demoSettings, handleSensorChange);
  return (
    <>

     <ValidatorForm onSubmit={onSubmit}>
        {demoSettings["sensors"].map((sensor,index) => (
    <div     key = {index}>
      
          <SensorControllerForm
          sensor = {sensor}
          handleSensorChange={handleSensorChange}
          handleRemoveSensor={handleRemoveSensor}
          index = {index}
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

function SensorControllerForm(props) {
  const { sensor, handleSensorChange, index ,handleRemoveSensor} = props;
  const classes = useStyles();

  return (
    <>
    <SensorsAttributeForm attributename="name" sensor={sensor} fieldType="string" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="min" sensor={sensor} fieldType="int" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="max" sensor={sensor} fieldType="int" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="unit" sensor={sensor} fieldType="string" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="enabled" sensor={sensor} fieldType="bool" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="driver" sensor={sensor} fieldType="driver" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="interval" sensor={sensor} fieldType="int" unit="sec" handleSensorChange={handleSensorChange} index={index}/>
    <Button color="secondary" className={classes.button} onClick={() => { handleRemoveSensor(index)}}>
        Remove
      </Button>

    </>
  );
}

function getValidator(type){
  var validators = ['required'];
  if (type=="int")
    validators.push('matchRegexp:^-?[0-9]+$');
  return validators;
}

function SensorsAttributeForm(props) {
  const { sensor, index, attributename, handleSensorChange,fieldType,unit } = props;
  const classes = useStyles();
  console.log("classes",classes);
  if (attributename=="driver"){
    return (
      <FormControl className={classes.textField} key={index}>
        <FormHelperText>{attributename}</FormHelperText>
      {/* <InputLabel id="demo-simple-select-label"></InputLabel> */}
      <Select
          name={"sensor_" + attributename + index}
          label={"Sensor " + attributename  }
          id={"sensor" + attributename + index}
          className={classes.textField}
          value={sensor[attributename]}//Why could this be undefined???
          onChange={ handleSensorChange(index,attributename)}
        >
          <MenuItem value="undefined">
            <em>Select a driver</em>
          </MenuItem>
        <MenuItem value={10}>Ten</MenuItem>
        <MenuItem value={20}>Twenty</MenuItem>
        <MenuItem value={30}>Thirty</MenuItem>
      </Select>
      </FormControl>
    )
  }
  return (
    <TextValidator 
      validators={getValidator(fieldType)}
      errorMessages={[attributename + ' is required']}
      name={"sensor_" + attributename + index}
      label={"Sensor " + attributename + (unit?" (in "+unit+")":'')}
      id={"sensor" + attributename + index}
      className={classes.textField}
      value={sensor[attributename]}//Why could this be undefined???
      onChange={ handleSensorChange(index,attributename)}
      margin="normal"
      />
  );
}




export default restComponent(DEMO_SETTINGS_ENDPOINT, DemoController);
