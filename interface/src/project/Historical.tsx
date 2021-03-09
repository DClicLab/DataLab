import React, { Component } from 'react';
import { Redirect, Switch, RouteComponentProps } from 'react-router-dom'

import { Tabs, Tab } from '@material-ui/core';

import { PROJECT_PATH } from '../api';
import { MenuAppBar } from '../components';
import { AuthenticatedRoute } from '../authentication';


import FileList from './FilesController'
import DataExplorer from './DataExplorer';
// import LightStateRestController from './LightStateRestController';
// import LightStateWebSocketController from './LightStateWebSocketController';
// import LightMqttSettingsController from './LightMqttSettingsController';
// import FileList from './FilesController';

class DataHistory extends Component<RouteComponentProps> {

  handleTabChange = (event: React.ChangeEvent<{}>, path: string) => {
    this.props.history.push(path);
  };

  render() {
    return (
      <MenuAppBar sectionTitle="Historical Data">
        <Tabs value={this.props.match.url} onChange={this.handleTabChange} variant="fullWidth">
        <Tab value={`/${PROJECT_PATH}/history/explorer`} label="data explorer" />
        <Tab value={`/${PROJECT_PATH}/history/files`} label="file manager" />
          {/* <Tab value={`/${PROJECT_PATH}/demo/rest`} label="REST Controller" />
          <Tab value={`/${PROJECT_PATH}/demo/socket`} label="WebSocket Controller" />
          <Tab value={`/${PROJECT_PATH}/demo/mqtt`} label="MQTT Controller" /> */}
          {/* <Tab value={`/${PROJECT_PATH}/demo/files`} label="Data files" /> */}
        </Tabs>
        <Switch>
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/history/explorer`} component={DataExplorer} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/history/files`} component={FileList} />
          {/* <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/files`} component={FileList} /> */}

          {/* <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/rest`} component={LightStateRestController} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/socket`} component={LightStateWebSocketController} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/demo/mqtt`} component={LightMqttSettingsController} /> */}
          <Redirect to={`/${PROJECT_PATH}/history/explorer`} />
        </Switch>
      </MenuAppBar>
    )
  }

}

export default DataHistory;
