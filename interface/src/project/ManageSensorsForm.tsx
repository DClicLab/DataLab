import React, { Fragment } from 'react';
import { ValidatorForm } from 'react-material-ui-form-validator';

import { Table, TableBody, TableCell, TableHead, TableFooter, TableRow, withWidth, WithWidthProps, isWidthDown, Box, Dialog, DialogTitle, DialogContent, DialogActions } from '@material-ui/core';
import { Button } from '@material-ui/core';

import EditIcon from '@material-ui/icons/Edit';
import DeleteSweepIcon from '@material-ui/icons/DeleteSweep';
import DeleteIcon from '@material-ui/icons/Delete';
import CloseIcon from '@material-ui/icons/Close';
import CheckIcon from '@material-ui/icons/Check';
import IconButton from '@material-ui/core/IconButton';
import SaveIcon from '@material-ui/icons/Save';
import CompassCalibration from '@material-ui/icons/CompassCalibration';

import { withAuthenticatedContext, AuthenticatedContextProps, redirectingAuthorizedFetch } from '../authentication';
import { RestFormProps, FormActions, FormButton, extractEventValue, ErrorButton } from '../components';

import SensorForm from './SensorForm';
import { SensorsSettings, Sensor } from './types';
import { ENDPOINT_ROOT } from '../api';




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
}

class ManageSensorsForm extends React.Component<ManageSensorsFormProps, ManageSensorsFormState> {
  
  state: ManageSensorsFormState = {
    confirmReset: false,
    processing: false,
    creating: false,
  };

  createSensor = () => {
    this.setState({
      creating: true,
      sensor: new Sensor()
    });
  };
  componentDidMount = ()=> {
    console.log("manage sensor mounted");
    console.log(this.props.data.sensors);
  };
  uniqueSensorname = (name: string) => {

    return this.props.data.sensors===null || !this.props.data.sensors.find(u => u.name === name);
  }

  // noAdminConfigured = () => {
  //   return !this.props.data.sensors.find(u => u.enabled);
  // }

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
    console.log("props before", this.props.data);
    
    if (sensor) {
      const { data } = this.props;
      const sensors = data.sensors===null ? [] :data.sensors.filter(u => u.name !== sensor.name);
      // sensors.push(sensor);
      sensors.push(JSON.parse(JSON.stringify(sensor)))//need a deep clone here
      this.props.setData({ ...data, sensors });
      this.setState({
        sensor: undefined
      });
    }
    console.log("props before", this.props.data);
  };

  handleSensorParamChange = (key: string) =>(event: React.ChangeEvent<HTMLInputElement>) => {
    let sensor ={ ...this.state.sensor!}
    let param = sensor.driver.config as Record<string, any>
    param[key]=extractEventValue(event) as string
    // param.set(key,  extractEventValue(event) as string)
    this.setState( {sensor})
  };

  handleSensorDriverChange = () => (event: React.ChangeEvent<HTMLInputElement>) => {
    // const csensor = { ...this.state.sensor}
    
    // csensor["driver"] = this.props.data.drivers[+event.target.value]
    // this.setState({ csensor })
    this.setState({ sensor: { ...this.state.sensor!, driver:  this.props.data.drivers[+event.target.value] } });
  };


  handleSensorValueChange = (name: keyof Sensor) => (event: React.ChangeEvent<HTMLInputElement>) => {
    this.setState({ sensor: { ...this.state.sensor!, [name]: extractEventValue(event) } });
  };

  onSubmit = () => {
    this.props.saveData();
    this.props.authenticatedContext.refresh();
  }
  secondsToHms = (d:number) => {
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
    redirectingAuthorizedFetch(ENDPOINT_ROOT+"resetConf", { method: 'POST' })
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
          Are you sure you want to delete all sensor configuration?<br/>The device will restart.
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
    const { sensor, creating } = this.state;

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


              {(data.sensors != null && data.sensors.length >0) ?
              data.sensors.sort(compareSensors).map(csensor => (
                <TableRow key={csensor.name}>
                  <TableCell component="th" scope="row">
                    {csensor.name}
                  </TableCell>
                  <TableCell align="center">
                    {
                      csensor.enabled? <CheckIcon /> : <CloseIcon />
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

                       {csensor.driver.config && Object.entries(csensor.driver.config as Object).map(([key ,value]) => {
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
          {
            // this.noAdminConfigured() &&
            // (
            //   <Box bgcolor="error.main" color="error.contrastText" p={2} mt={2} mb={2}>
            //     <Typography variant="body1">
            //       You must have at least one admin sensor configured.
            //     </Typography>
            //   </Box>
            // )
          }
          <FormActions>
            {/* <FormButton startIcon={<SaveIcon />} variant="contained" color="primary" type="submit" disabled={this.noAdminConfigured()}> */}
            <FormButton startIcon={<SaveIcon />} variant="contained" color="primary" type="submit" >
              Save
            </FormButton>              
            
        {this.props.authenticatedContext.me.admin?
            <ErrorButton startIcon={<DeleteSweepIcon />} variant="contained" onClick={this.onDeleteAll}>
                Reset configuration
              </ErrorButton>
              :""}
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
