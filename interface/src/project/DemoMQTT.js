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


class DemoMQTT extends Component {
  componentDidMount() {
    this.props.loadData();
  }

  componentDidUpdate(){
    console.log("got an update",this.props.data);
  }
  
  render() {
    const { data, fetched, errorMessage, saveData, loadData, handleNewMQTT, handleCloudChange, handleRemoveMQTT } = this.props;
    
    return (
      <SectionContent title="Controller" titleGutter>
        <LoadingNotification
          onReset={loadData}
          fetched={fetched}
          errorMessage={errorMessage}
          render={() =>
            <MQTTForm
              demoSettings={data}
              onReset={loadData}
              onSubmit={saveData}
              handleCloudChange={handleCloudChange}
              handleRemoveMQTT={handleRemoveMQTT}
            />

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
}));


function MQTTForm(props) {
  const { demoSettings, onSubmit, onReset, handleCloudChange, handleRemoveMQTT } = props;
  const classes = useStyles();

  return (
    <>

     <ValidatorForm onSubmit={onSubmit}>      
          <MQTTControllerForm
          MQTT = {demoSettings["cloudService"]}
          handleCloudChange={handleCloudChange}
          handleRemoveMQTT={handleRemoveMQTT}
          />
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

function MQTTControllerForm(props) {
  const { MQTT, handleCloudChange, index ,handleRemoveMQTT} = props;
  const classes = useStyles();

  return (
    <>
    <MQTTsAttributeForm attributename="driver" MQTT={MQTT} fieldType="driver" handleCloudChange={handleCloudChange} index={index}/>
    <MQTTsAttributeForm attributename="host" MQTT={MQTT} fieldType="string" handleCloudChange={handleCloudChange} index={index}/>
    <MQTTsAttributeForm attributename="credentials" MQTT={MQTT} fieldType="string" handleCloudChange={handleCloudChange} index={index}/>
    <MQTTsAttributeForm attributename="format" MQTT={MQTT} fieldType="string" handleCloudChange={handleCloudChange} index={index}/>
    <MQTTsAttributeForm attributename="target" MQTT={MQTT} fieldType="string" handleCloudChange={handleCloudChange} index={index}/>
    <Button color="secondary" className={classes.button} onClick={() => { handleRemoveMQTT(index)}}>
        Remove
      </Button>

    </>
  );
}

function getValidator(type){
  var validators = [];
  if (type=="int")
    validators.push('matchRegexp:^-?[0-9]+$');
  return validators;
}

function MQTTsAttributeForm(props) {
  const { MQTT, index, attributename, handleCloudChange,fieldType,unit } = props;
  const classes = useStyles();
  console.log("classes",classes);
  if (attributename=="driver"){
    return (
      <FormControl className={classes.textField} key={index}>
        <FormHelperText>{attributename}</FormHelperText>
      {/* <InputLabel id="demo-simple-select-label"></InputLabel> */}
      <Select
          name={"MQTT_" + attributename}
          label={"MQTT " + attributename  }
          id={"MQTT" + attributename }
          className={classes.textField}
          value={MQTT[attributename]}
          onChange={ handleCloudChange(attributename)}
        >
          <MenuItem value="undefined">
            <em>Select a driver</em>
          </MenuItem>
        <MenuItem value={"MQTT"}>MQTT</MenuItem>
        <MenuItem value={"HTTP"}>HTTP</MenuItem>
      </Select>
      </FormControl>
    )
  }
  return (
    <TextValidator 
      validators={getValidator(fieldType)}
      errorMessages={[attributename + ' is required']}
      name={"MQTT_" + attributename + index}
      label={"Cloud " + attributename + (unit?" (in "+unit+")":'')}
      id={"MQTT" + attributename + index}
      className={classes.textField}
      value={MQTT[attributename]}//Why could this be undefined???
      onChange={ handleCloudChange(attributename)}
      margin="normal"
      />
  );
}




export default restComponent(DEMO_SETTINGS_ENDPOINT, DemoMQTT);
