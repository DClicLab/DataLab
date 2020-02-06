import React, { Component } from 'react';
import { Redirect, Switch } from 'react-router-dom'

import { PROJECT_PATH } from '../constants/Env';
import MenuAppBar from '../components/MenuAppBar';
import AuthenticatedRoute from '../authentication/AuthenticatedRoute';
import DataInformation from './DataInformation';
import DataController from './DataController';
import DataMQTT from './DataMQTT';

import Tabs from '@material-ui/core/Tabs';
import Tab from '@material-ui/core/Tab';
import { withSnackbar } from 'notistack';
import { ENDPOINT_ROOT } from '../constants/Env';


export const Data_SETTINGS_ERRORS = ENDPOINT_ROOT + "../getErrors";



class DataProject extends Component {

  handleTabChange = (event, path) => {
    this.props.history.push(path);
  };


  componentDidMount() {
    //    this.props.loadData();
      //   window.setInterval(() => {
      //     fetch(Data_SETTINGS_ERRORS)
      //     .then((res) => this.props.enqueueSnackbar(res))
      //   }, 2000)
      // }

  }

  render() {
    return (
      <MenuAppBar sectionTitle="Data Project">
        <Tabs value={this.props.match.url} onChange={this.handleTabChange} indicatorColor="primary" textColor="primary" variant="fullWidth">
          <Tab value={`/${PROJECT_PATH}/Data/information`} label="Monitoring" />
          <Tab value={`/${PROJECT_PATH}/Data/controller`} label="Sensor" />
          <Tab value={`/${PROJECT_PATH}/Data/cloudservice`} label="Cloud service" />
        </Tabs>
        <Switch>
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/Data/information`} component={DataInformation} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/Data/controller`} component={DataController} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/Data/cloudservice`} component={DataMQTT} />
          <Redirect to={`/${PROJECT_PATH}/Data/information`}  />
        </Switch>
      </MenuAppBar>
    )
  }        

}
export default withSnackbar(DataProject);