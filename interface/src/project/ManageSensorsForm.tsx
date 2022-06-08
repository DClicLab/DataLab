import React, { Fragment } from 'react';
import { SelectValidator, TextValidator, ValidatorForm } from 'react-material-ui-form-validator';

import { Table, TableBody, TableCell, TableHead, TableFooter, TableRow, withWidth, WithWidthProps, isWidthDown, Box, Dialog, DialogTitle, DialogContent, DialogActions, Checkbox, Typography, Divider, FormControlLabel, Switch, MenuItem, Grid, Select, FormHelperText, FormControl, Badge } from '@material-ui/core';
import { Button } from '@material-ui/core';

import EditIcon from '@material-ui/icons/Edit';
import DeleteSweepIcon from '@material-ui/icons/DeleteSweep';
import DeleteIcon from '@material-ui/icons/Delete';
import CloseIcon from '@material-ui/icons/Close';
import CheckIcon from '@material-ui/icons/Check';
import ErrorIcon from '@material-ui/icons/Error';
import IconButton from '@material-ui/core/IconButton';
import SaveIcon from '@material-ui/icons/Save';
import CompassCalibration from '@material-ui/icons/CompassCalibration';

import { withAuthenticatedContext, AuthenticatedContextProps, redirectingAuthorizedFetch } from '../authentication';
import { RestFormProps, FormActions, FormButton, extractEventValue, ErrorButton, BlockFormControlLabel } from '../components';

import SensorForm from './SensorForm';
import { SensorsSettings, Sensor, I2CConf } from './types';
import { ENDPOINT_ROOT } from '../api';
import { Block, DepartureBoard } from '@material-ui/icons';
import { threadId } from 'node:worker_threads';




function compareSensors(a: Sensor, b: Sensor) {
  if (a.name < b.name) {
    return -1;
  }
  if (a.name > b.name) {
    return 1;
  }
  return 0;
}

type ManageSensorsFormProps = RestFormProps<SensorsSettings> & AuthenticatedContextProps & WithWidthProps;

type ManageSensorsFormState = {
  creating: boolean;
  processing: boolean;
  confirmReset: boolean;
  sensor?: Sensor;
  driverlist?: string[];
  i2cmissing: boolean;
}

class ManageSensorsForm extends React.Component<ManageSensorsFormProps, ManageSensorsFormState> {

  state: ManageSensorsFormState = {
    confirmReset: false,
    processing: false,
    creating: false,
    i2cmissing: false
  };

  createSensor = () => {
    this.setState({
      creating: true,
      sensor: new Sensor()
    });
  };
  componentDidMount = () => {
    console.log("manage sensor mounted");
    console.log(this.props.data.sensors);
  };

  uniqueSensorname = (name: string) => {
    return this.props.data.sensors === null || !this.props.data.sensors.find(u => u.name === name);
  }
  
  isi2cmissing = ()=>{
    if (!this.props.data.i2c.enabled && this.props.data.sensors!=undefined){
      var a= this.props.data.sensors.some(item=>(item.driver.i2c != undefined)&& item.driver.i2c==1);
      return a;
    }
    return false;
  }

  removeSensor = (sensor: Sensor) => {
    const { data } = this.props;
    const sensors = data.sensors.filter(u => u.name !== sensor.name);
    this.props.setData({ ...data, sensors });
  }

  startEditingSensor = (sensor: Sensor) => {
    this.setState({
      creating: false,
      sensor: sensor
    });
  };

  cancelEditingSensor = () => {
    this.setState({
      sensor: undefined
    });
  }

  doneEditingSensor = () => {
    const { sensor } = this.state;

    if (sensor) {
      const { data } = this.props;
      const sensors = data.sensors === null ? [] : data.sensors.filter(u => u.name !== sensor.name);
      // sensors.push(sensor);
      sensors.push(JSON.parse(JSON.stringify(sensor)))//need a deep clone here
      this.props.setData({ ...data, sensors });
      this.setState({i2cmissing: this.isi2cmissing()});
      this.setState({
        sensor: undefined
      });
    }
  };

  handleSensorParamChange = (key: string) => (event: React.ChangeEvent<HTMLInputElement>) => {
    let sensor = { ...this.state.sensor! }
    let param = sensor.driver.config as Record<string, any>
    param[key] = extractEventValue(event) as string
    // param.set(key,  extractEventValue(event) as string)
    this.setState({ sensor })
  };

  handleSensorDriverChange = () => (event: React.ChangeEvent<HTMLInputElement>) => {
    // const csensor = { ...this.state.sensor}

    // csensor["driver"] = this.props.data.drivers[+event.target.value]
    // this.setState({ csensor })
    this.setState({ sensor: { ...this.state.sensor!, driver: this.props.data.drivers[+event.target.value] } });
  };


  handleSensorValueChange = (name: keyof Sensor) => (event: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({ sensor: { ...this.state.sensor!, [name]: extractEventValue(event) } });
  };



  handleI2CValueChange = (name: keyof I2CConf) => (event: React.ChangeEvent<HTMLInputElement>) => {
    let i2c = this.props.data.i2c;
    //@ts-ignore
    i2c[name]=extractEventValue(event);
    this.props.setData({...this.props.data,i2c});
    this.setState({i2cmissing: this.isi2cmissing()});

  };

  updatePorts = () => (event: React.ChangeEvent<HTMLInputElement>) => {
    // this.handleI2CValueChange('conf')(event);
    const i2c = this.props.data.i2c;
    i2c.conf = extractEventValue(event) as string;
    console.log("Updating ports");
    var conf = extractEventValue(event);
    switch (conf) {
      case "HAT":
        console.log("hat")
        i2c.sda = 0;
        i2c.scl = 26;
        break;
      case "GROVE":
        i2c.sda = 32;
        i2c.scl = 33;
        break;
      case "Custom":
        i2c.sda = -1;
        i2c.scl = -1;
        break;
     default:
        break;
    }
    this.props.setData({...this.props.data,i2c});
  };

  onSubmit = () => {
    this.props.saveData();
    this.props.authenticatedContext.refresh();
  }
  secondsToHms = (d: number) => {
    d = Number(d);
    var h = Math.floor(d / 3600);
    var m = Math.floor(d % 3600 / 60);
    var s = Math.floor(d % 3600 % 60);

    var hDisplay = h > 0 ? h + (h === 1 ? " hour " : " hours ") : "";
    var mDisplay = m > 0 ? m + (m === 1 ? " minute " : " minutes ") : "";
    var sDisplay = s > 0 ? s + (s === 1 ? " second" : " seconds") : "";
    return hDisplay + mDisplay + sDisplay;
  }

  onDeleteAll = () => {
    this.setState({ confirmReset: true });
  }

  onDeleteAllRejected = () => {
    this.setState({ confirmReset: false });
  }

  onDeleteAllConfirmed = () => {
    this.setState({ processing: true });
    redirectingAuthorizedFetch(ENDPOINT_ROOT + "resetconf", { method: 'POST' })
      .then(response => {
        if (response.status === 200) {
          this.props.enqueueSnackbar("Delete all in progress. Refreshing page in 5 sec.", { variant: 'error' });
          this.setState({ processing: false, confirmReset: false });
          setTimeout(() => {
            window.location.reload();
          }, 5000);
        } else {
          throw Error("Invalid status code: " + response.status);
        }
      })
      .catch(error => {
        this.props.enqueueSnackbar(error.message || "Problem factory resetting device", { variant: 'error' });
        this.setState({ processing: false });
      });
  }
  renderDeleteAllDialog() {
    return (
      <Dialog
        open={this.state.confirmReset}
        onClose={this.onDeleteAllRejected}
      >
        <DialogTitle>Confirm Delete All</DialogTitle>
        <DialogContent dividers>
          Are you sure you want to delete all sensor configuration?<br />The device will restart.
        </DialogContent>
        <DialogActions>
          <Button variant="contained" onClick={this.onDeleteAllRejected} color="secondary">
            Cancel
          </Button>
          <ErrorButton startIcon={<DeleteSweepIcon />} variant="contained" onClick={this.onDeleteAllConfirmed} disabled={this.state.processing} autoFocus>
            Reset
          </ErrorButton>
        </DialogActions>
      </Dialog>
    )
  }

  render() {
    const { width, data } = this.props;
    if (data.sensors === undefined) {
      data.sensors = [];
    }
    if (data.i2c === undefined) {
      data.i2c=new I2CConf;
      data.i2c.enabled = true;
      data.i2c.conf = "Custom";
      data.i2c.sda = -1;
      data.i2c.scl = -1;
    }
    const { sensor, creating,i2cmissing } = this.state;

    let table = <TableRow><TableCell colSpan={6}>No sensor defined</TableCell></TableRow>;

    return (
      <Fragment>
        <ValidatorForm onSubmit={this.onSubmit}>
          <Table size="small" padding={isWidthDown('xs', width!) ? "none" : "default"}>
            <TableHead>
              <TableRow>
                <TableCell>Name</TableCell>
                <TableCell align="center">Enabled</TableCell>
                <TableCell align="center">Interval</TableCell>
                <TableCell align="center">Driver</TableCell>
                <TableCell align="center">Params</TableCell>
                <TableCell />
              </TableRow>
            </TableHead>
            <TableBody>


              {(data.sensors != null && data.sensors.length > 0) ?
                data.sensors.sort(compareSensors).map(csensor => (
                  <TableRow key={csensor.name}>
                    <TableCell component="th" scope="row">
                    <Badge badgeContent={csensor.driver.i2c?"I2C":""}>
                      {csensor.name}
                    </Badge>
                    </TableCell>
                    <TableCell align="center">
                      {
                        csensor.enabled ? <CheckIcon /> : <CloseIcon />
                      }
                    </TableCell>
                    <TableCell component="th" align="center" scope="row">
                      {this.secondsToHms(csensor.interval)}
                    </TableCell>
                    <TableCell component="th" scope="row">
                      {csensor.driver?.name}
                    </TableCell>
                    <TableCell component="th" scope="row">
                      <Table size="small" >
                        <TableBody>
                          {//@ts-ignore
                            <TableRow>

                              {csensor.driver.config && Object.entries(csensor.driver.config as Object).map(([key, value]) => {
                                return <TableCell key={csensor.name + key}>{key}: {value}</TableCell>
                              })}

                            </TableRow>
                          }
                        </TableBody>
                      </Table>
                    </TableCell>

                    <TableCell align="center">
                      <IconButton size="small" aria-label="Delete" onClick={() => this.removeSensor(csensor)}>
                        <DeleteIcon />
                      </IconButton>
                      <IconButton size="small" aria-label="Edit" onClick={() => this.startEditingSensor(csensor)}>
                        <EditIcon />
                      </IconButton>
                    </TableCell>
                  </TableRow>
                ))
                : table
              }
            </TableBody>
            <TableFooter >
              <TableRow>
                <TableCell colSpan={5} />
                <TableCell align="center" padding="default">
                  <Button startIcon={<CompassCalibration />} variant="contained" color="secondary" onClick={this.createSensor}>
                    Add Sensor
                  </Button>
                </TableCell>
              </TableRow>
            </TableFooter>
          </Table>

          <Divider></Divider>

          <Box component="span" m={3}>
            <Typography variant="h6">I2C Bus settings </Typography>
            <Grid container spacing={3}>
              <Grid item xs={3}>
                <FormControlLabel
                  control={
                    <Switch
                      checked={data.i2c.enabled}
                      onChange={this.handleI2CValueChange('enabled')}
                      name="i2c_enabled"
                      color="primary"
                    />
                  }
                  label={data.i2c.enabled?"Enabled":"Disabled"}
                />
              </Grid>
              <Grid item xs={3}>
                <FormControl>
                  <SelectValidator value={data.i2c.conf} name="i2cbus" disabled={!data.i2c.enabled} onChange={this.updatePorts()}>
                    <MenuItem value="HAT">HAT</MenuItem>
                    <MenuItem value="GROVE">GROVE</MenuItem>
                    <MenuItem value="Custom">Custom</MenuItem>
                  </SelectValidator>
                  <FormHelperText>Port Configuration</FormHelperText>
                </FormControl>
              </Grid>
              <Grid item xs={3}>
                <FormControl >
                  <SelectValidator disabled={data.i2c.conf != 'Custom'} value={data.i2c.sda} name="i2csda" onChange={this.handleI2CValueChange('sda')}>
                    <MenuItem key="-1" value="-1">Default</MenuItem>
                    {Array.from(Array(40).keys()).map((num, idx) => {
                      return <MenuItem key={idx} disabled={data.i2c.scl == num} value={num}>{num}</MenuItem>
                    })}
                  </SelectValidator>
                  <FormHelperText>SDA Pin</FormHelperText>
                </FormControl>
              </Grid>
              <Grid item xs={3}>
                <FormControl >
                  <SelectValidator disabled={data.i2c.conf != 'Custom'} value={data.i2c.scl} name="i2cscl" onChange={this.handleI2CValueChange('scl')}>
                    <MenuItem key='-1' value="-1">Default</MenuItem>
                    {Array.from(Array(40).keys()).map((num, idx) => {
                      return <MenuItem key={idx} disabled={data.i2c.sda == num} value={num}>{num}</MenuItem>
                    })}
                  </SelectValidator>
                  <FormHelperText>SCL Pin</FormHelperText>
                </FormControl>
              </Grid>

            </Grid>
          </Box>
          
          <Box hidden={!this.isi2cmissing()}>
          <Grid container>
                    <Grid><ErrorIcon color="error"/></Grid>
                    <Grid><Typography>Error, at least one sensor needs I2C bus. Enable and configure I2C, or remove the I2C driver.</Typography></Grid>
          </Grid>
             </Box>
          <Divider/>

          <FormActions textAlign="center">
            {/* <FormButton startIcon={<SaveIcon />} variant="contained" color="primary" type="submit" disabled={this.noAdminConfigured()}> */}
            <FormButton startIcon={<SaveIcon />} disabled={this.isi2cmissing()} variant="contained" color="primary" type="submit" >
              Save
            </FormButton>

            {this.props.authenticatedContext.me.admin ?
              <ErrorButton startIcon={<DeleteSweepIcon />} variant="contained" onClick={this.onDeleteAll}>
                Reset configuration
              </ErrorButton>
              : ""}
          </FormActions>
        </ValidatorForm>
        {
          sensor &&
          <SensorForm
            sensor={sensor}
            driverlist={data.drivers}
            creating={creating}
            onDoneEditing={this.doneEditingSensor}
            onCancelEditing={this.cancelEditingSensor}
            handleDriverChange={this.handleSensorDriverChange}
            handleParamChange={this.handleSensorParamChange}
            handleValueChange={this.handleSensorValueChange}
            uniqueSensorname={this.uniqueSensorname}
          />
        }
        {this.renderDeleteAllDialog()}
      </Fragment>
    );
  }

}

export default withAuthenticatedContext(withWidth()(ManageSensorsForm));
