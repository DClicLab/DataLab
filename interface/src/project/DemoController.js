import React, { Component } from 'react';
import { ValidatorForm,TextValidator } from 'react-material-ui-form-validator';

import { ENDPOINT_ROOT } from '../constants/Env';
import SectionContent from '../components/SectionContent';
import { restComponent } from '../components/RestComponent';
import LoadingNotification from '../components/LoadingNotification';
import { withSnackbar } from 'notistack';
import { green } from '@material-ui/core/colors';
import FormGroup from '@material-ui/core/FormGroup';
import Button from '@material-ui/core/Button';
import { makeStyles } from '@material-ui/core/styles';
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/MenuItem';
import FormHelperText from '@material-ui/core/FormHelperText';
import FormControl from '@material-ui/core/FormControl';
import SelectValidator from '@material-ui/core/Select';

import FormControlLabel from '@material-ui/core/FormControlLabel';
import Checkbox from '@material-ui/core/Checkbox';
import CheckboxValidator from '@material-ui/core/Checkbox';

import { array } from 'prop-types';


export const DEMO_SETTINGS_ENDPOINT = ENDPOINT_ROOT + "demoSettings";



const useStyles = makeStyles(theme => ({
  button: {
    marginRight: theme.spacing(1),
    marginTop: theme.spacing(2),
  },
  selectField: {
    // width: "100%",
    color: "blue",
    marginTop: theme.spacing(1.3),
    marginBottom: theme.spacing(0.5)
  },
  blinkSpeedLabel: {
    marginBottom: theme.spacing(5),
  }
}));

return withSnackbar(
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
            <FormGroup row>
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

          </FormGroup>
          }
        />
      </SectionContent>
    )
  }
}
);



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
          demoSettings = {demoSettings}
          handleSensorChange={handleSensorChange}
          handleRemoveSensor={handleRemoveSensor}
          index = {index}
          key = {sensor.name}
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
  const { sensor, handleSensorChange, index ,handleRemoveSensor, demoSettings} = props;
  const classes = useStyles();

  return (
    <>
    <SensorsAttributeForm attributename="enabled" sensor={sensor} fieldType="bool" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="name" sensor={sensor} fieldType="string" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="driver" sensor={sensor}  fieldType="driver" demoSettings={demoSettings} handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="interval" sensor={sensor} fieldType="int" unit="sec" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="config" sensor={sensor} fieldType="config" handleSensorChange={handleSensorChange} index={index}/>
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
  const { sensor, index, attributename, handleSensorChange,fieldType,demoSettings } = props;
  const classes = useStyles();
  console.log("classes",classes);
  if (attributename=="config"){
    return Object.keys(sensor[attributename]).map((key,cindex)=> (
            
      <TextValidator 
      name={"sensor_" + attributename + index+cindex}
      label={"Configuration: " + key }
      id={"sensor" + attributename + index + cindex}
      className={classes.textField}
      value={sensor[attributename][key]}//Why could this be undefined???
      onChange={ handleSensorChange(index,attributename,key)}
      margin="normal"
      disabled = {!sensor["enabled"]}
      />
        
    
    
    ))


  }


  if (attributename=="driver"){
    return (
      <FormControl       disabled = {!sensor["enabled"]}
      >
        <FormHelperText>{attributename}</FormHelperText>
      {/* <InputLabel id="demo-simple-select-label"></InputLabel> */}
      <SelectValidator
          name={"sensor_" + attributename + index}
          label={"Sensor " + attributename  }
          id={"sensor" + attributename + index}
          className={classes.selectField}
          value={sensor[attributename]}//Why could this be undefined???
          onChange={ handleSensorChange(index,attributename,demoSettings)}
        >
          <MenuItem value="undefined">
            <em>Select a driver</em>
          </MenuItem>
          {demoSettings["drivers"].map((driverdesc)=> (
            
            <MenuItem key={driverdesc.name} value={driverdesc.name}>{driverdesc.name}</MenuItem>
          
          ))}
      </SelectValidator>
      </FormControl>
    )
  }
  if (fieldType === "bool"){
    return(
      <FormControlLabel
      control={
        <Checkbox
        name={"sensor_" + attributename + index}
        label={"Sensor " + attributename }
        id={"sensor" + attributename + index}
        className={classes.textField}
        checked={sensor[attributename]}
        onChange={handleSensorChange(index,attributename)}
        margin="normal"
        color="primary"
        />
      }
      label="Enabled"
    />
    )
  }
  return (
    <TextValidator 
      validators={getValidator(fieldType)}
      errorMessages={[attributename + ' is required']}
      name={"sensor_" + attributename + index}
      label={"Sensor " + attributename }
      id={"sensor" + attributename + index}
      className={classes.textField}
      value={sensor[attributename]}//Why could this be undefined???
      onChange={ handleSensorChange(index,attributename)}
      margin="normal"
      disabled = {!sensor["enabled"]}
      />
  );
}
export default restComponent(DEMO_SETTINGS_ENDPOINT, DemoController);
