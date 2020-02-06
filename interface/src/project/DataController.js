import React, { Component } from 'react';
import { ValidatorForm,TextValidator } from 'react-material-ui-form-validator';

import { ENDPOINT_ROOT } from '../constants/Env';
import SectionContent from '../components/SectionContent';
import { restComponent } from '../components/RestComponent';
import LoadingNotification from '../components/LoadingNotification';

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


export const DATA_SETTINGS_ENDPOINT = ENDPOINT_ROOT + "dataSettings";



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


class DataController extends Component {
  componentDidMount() {
    this.props.loadData();
  }

  componentDidUpdate(){
    console.log("got an update",this.props.data);
  }
  
  render() {
    const { data, fetched, errorMessage, saveData, loadData, handleNewSensor, handleSensorChange, handleRemoveSensor } = this.props;
    
    return (
      <SectionContent title="Sensors" titleGutter>
     <Button color="primary" variant="contained" onClick={handleNewSensor}>
        Add a sensor
      </Button>

        <LoadingNotification
          onReset={loadData}
          fetched={fetched}
          errorMessage={errorMessage}
          render={() =>
            <FormGroup row>
            {/* <DataControllerForm
              dataSettings={data}
              onReset={loadData}
              onSubmit={saveData}
              handleSliderChange={handleSliderChange}
            /> */}
            <SensorsControllerForm
              dataSettings={data}
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



function SensorsControllerForm(props) {
  const { dataSettings, onSubmit, onReset, handleSensorChange,handleRemoveSensor } = props;
  const classes = useStyles();

  console.log("We have ",dataSettings, handleSensorChange);
  return (
    <>

     <ValidatorForm onSubmit={onSubmit}>
        {dataSettings["sensors"].map((sensor,index) => (
    <div     key = {index}>
      
          <SensorControllerForm
          sensor = {sensor}
          dataSettings = {dataSettings}
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
  const { sensor, handleSensorChange, index ,handleRemoveSensor, dataSettings} = props;
  const classes = useStyles();

  return (
    <>
    <SensorsAttributeForm attributename="enabled" sensor={sensor} fieldType="bool" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="name" sensor={sensor} fieldType="string" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="driver" sensor={sensor}  fieldType="driver" dataSettings={dataSettings} handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="interval" sensor={sensor} fieldType="int" unit="sec" handleSensorChange={handleSensorChange} index={index}/>
    <SensorsAttributeForm attributename="config" sensor={sensor} fieldType="config" dataSettings={dataSettings}  handleSensorChange={handleSensorChange} index={index}/>
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
  const { sensor, index, attributename, handleSensorChange,fieldType,dataSettings } = props;
  const classes = useStyles();
  console.log("classes",classes);
  if (attributename=="config"){
    if (sensor["config"]==undefined){
        sensor["config"]=dataSettings["drivers"].filter(obj => {
          return obj.name === sensor["driver"]
        })[0]["conf"];
    }
    return Object.keys(sensor[attributename]).map((key,cindex)=> (
            
      <TextValidator 
      name={"sensor_" + attributename + index+cindex}
      label={"Configuration: " + key }
      id={"sensor" + attributename + index + cindex}
      key={"sensor" + attributename + index + cindex}
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
      {/* <InputLabel id="data-simple-select-label"></InputLabel> */}
      <SelectValidator
          name={"sensor_" + attributename + index}
          label={"Sensor " + attributename  }
          id={"sensor" + attributename + index}
          className={classes.selectField}
          value={sensor[attributename]}//Why could this be undefined???
          onChange={ handleSensorChange(index,attributename,dataSettings)}
        >
          <MenuItem value="undefined">
            <em>Select a driver</em>
          </MenuItem>
          {dataSettings["drivers"].map((driverdesc)=> (
            
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
export default restComponent(DATA_SETTINGS_ENDPOINT, DataController);
