import React, { Component } from 'react';
import { Redirect, Switch } from 'react-router-dom'

import { PROJECT_PATH } from '../constants/Env';
import MenuAppBar from '../components/MenuAppBar';
import AuthenticatedRoute from '../authentication/AuthenticatedRoute';
import DemoInformation from './DemoInformation';
import DemoController from './DemoController';
import DemoMQTT from './DemoMQTT';

import Tabs from '@material-ui/core/Tabs';
import Tab from '@material-ui/core/Tab';

class DemoProject extends Component {

  handleTabChange = (event, path) => {
    this.props.history.push(path);
  };

  render() {
    return (
      <MenuAppBar sectionTitle="Demo Project">
        <Tabs value={this.props.match.url} onChange={this.handleTabChange} indicatorColor="primary" textColor="primary" variant="fullWidth">
          <Tab value={`/${PROJECT_PATH}/demo/information`} label="Information" />
          <Tab value={`/${PROJECT_PATH}/demo/controller`} label="Controller" />
          <Tab value={`/${PROJECT_PATH}/demo/cloudservice`} label="Cloud service" />
        </Tabs>
        <Switch>
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/information`} component={DemoInformation} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/controller`} component={DemoController} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/cloudservice`} component={DemoMQTT} />
          <Redirect to={`/${PROJECT_PATH}/demo/information`}  />
        </Switch>
      </MenuAppBar>
    )
  }        

}

export default DemoProject;
