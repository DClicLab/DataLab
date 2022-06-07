import React, { Component } from 'react';
import { Redirect, Switch, RouteComponentProps } from 'react-router-dom'

import { Tabs, Tab } from '@material-ui/core';

import { PROJECT_PATH } from '../api';
import { MenuAppBar } from '../components';
import { AuthenticatedRoute } from '../authentication';

import DemoInformation from './DemoInformation';
import ManageSensorsController from './ManageSensorsController';
// import LightStateRestController from './LightStateRestController';
// import LightStateWebSocketController from './LightStateWebSocketController';
// import LightMqttSettingsController from './LightMqttSettingsController';
// import FileList from './FilesController';

class DemoProject extends Component<RouteComponentProps> {

  handleTabChange = (event: React.ChangeEvent<{}>, path: string) => {
    this.props.history.push(path);
  };

  render() {
    return (
      <MenuAppBar sectionTitle="DataLab">
        <Tabs value={this.props.match.url} onChange={this.handleTabChange} variant="fullWidth">
        <Tab value={`/${PROJECT_PATH}/demo/information`} label="Live view" />
        <Tab value={`/${PROJECT_PATH}/demo/sensors`} label="sensors" />
          {/* <Tab value={`/${PROJECT_PATH}/demo/rest`} label="REST Controller" />
          <Tab value={`/${PROJECT_PATH}/demo/socket`} label="WebSocket Controller" />
          <Tab value={`/${PROJECT_PATH}/demo/mqtt`} label="MQTT Controller" /> */}
          {/* <Tab value={`/${PROJECT_PATH}/demo/files`} label="Data files" /> */}
        </Tabs>
        <Switch>
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/information`} component={DemoInformation} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/sensors`} component={ManageSensorsController} />
          {/* <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/files`} component={FileList} /> */}

          {/* <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/rest`} component={LightStateRestController} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/socket`} component={LightStateWebSocketController} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/mqtt`} component={LightMqttSettingsController} /> */}
          <Redirect to={`/${PROJECT_PATH}/demo/information`} />
        </Switch>
      </MenuAppBar>
    )
  }

}

export default DemoProject;
