import React, { RefObject } from 'react';
import { TextValidator, ValidatorForm, SelectValidator } from 'react-material-ui-form-validator';

import { Dialog, DialogTitle, DialogContent, DialogActions, Checkbox,MenuItem, Typography } from '@material-ui/core';

import { BlockFormControlLabel, FormButton } from '../components';

import { Driver, Sensor } from './types';

interface SensorFormProps {
  creating: boolean;
  sensor: Sensor;
  driverlist: Driver[];
  uniqueSensorname: (value: any) => boolean;
  handleValueChange: (name: keyof Sensor) => (event: React.ChangeEvent<HTMLInputElement>) => void;
  handleParamChange: (key: string) => (event: React.ChangeEvent<HTMLInputElement>) => void;
  handleDriverChange: () => (event: React.ChangeEvent<HTMLInputElement>) => void;
  onDoneEditing: () => void;
  onCancelEditing: () => void;
}

class SensorForm extends React.Component<SensorFormProps> {

  formRef: RefObject<any> = React.createRef();

  componentDidMount() {
    ValidatorForm.addValidationRule('uniqueSensorname', this.props.uniqueSensorname);
  }

  submit = () => {
    this.formRef.current.submit();
  }
  

  render() {
    const { driverlist, sensor, creating, handleValueChange, handleDriverChange, handleParamChange, onDoneEditing, onCancelEditing } = this.props;
    return (
      <ValidatorForm onSubmit={onDoneEditing} ref={this.formRef}>
        <Dialog onClose={onCancelEditing} aria-labelledby="sensor-form-dialog-title" open>
          <DialogTitle id="sensor-form-dialog-title">{creating ? 'Add' : 'Modify'} Sensor</DialogTitle>
          <DialogContent dividers>
            <TextValidator
              validators={creating ? ['required', 'uniqueSensorname', 'matchRegexp:^[a-zA-Z0-9_\\.]{1,24}$'] : []}
              errorMessages={creating ? ['Sensorname is required', "Sensorname already exists", "Must be 1-24 characters: alpha numeric, '_' or '.'"] : []}
              name="name"
              label="Sensor name"
              disabled={!creating}
              fullWidth
              variant="outlined"
              value={sensor.name}
              // disabled={!creating}
              onChange={handleValueChange('name')}
              margin="normal"
              />
            <SelectValidator
          validators={['required']}
          errorMessages={['Driver is required']}
          name="driver"
          label="Driver"
          fullWidth
          variant="outlined"
          native="true"
          // value={driverlist.indexOf(sensor.driver)}
          value={  driverlist.map(e => e.name).indexOf(sensor.driver.name)}
          onChange={handleDriverChange()}
          margin="normal"
        >
          {/* <MenuItem disabled>Driver...</MenuItem> */}
          {driverlist.map((key,index:number) => (
    <MenuItem key={sensor.name + key.name} value={index}>{key.name}</MenuItem>
          ))}

        </SelectValidator>
          {
          
          sensor.driver.config && Object.entries(sensor.driver.config as Object).map(([key ,value]) => {
           return <TextValidator label={key} name={sensor.name+key} key={sensor.name+key} value={value} onChange={handleParamChange(key)} />
          })
          
          
          }
          <Typography>Note: SDA, SDC pins available on M5Stick: 0,26,32,33</Typography>
            <TextValidator
              validators={['required', 'minNumber:1', 'maxNumber:65000'] }
              name="interval"
              label="Sensor interval"
              fullWidth
              variant="outlined"
              value={sensor.interval}
              onChange={handleValueChange('interval')}
              margin="normal"
              />

            <BlockFormControlLabel
              control={
                <Checkbox
                  value="enabled"
                  checked={sensor.enabled}
                  onChange={handleValueChange('enabled')}
                />
              }
              label="Enabled"
            />
          </DialogContent>
          <DialogActions>
            <FormButton variant="contained" color="secondary" onClick={onCancelEditing}>
              Cancel
            </FormButton>
            <FormButton variant="contained" color="primary" type="submit" onClick={this.submit}>
              Done
            </FormButton>
          </DialogActions>
        </Dialog>
      </ValidatorForm>
    );
  }
}

export default SensorForm;
