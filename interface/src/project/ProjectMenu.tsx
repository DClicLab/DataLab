import React, { Component } from 'react';
import { Link, withRouter, RouteComponentProps } from 'react-router-dom';

import {List, ListItem, ListItemIcon, ListItemText} from '@material-ui/core';
import SettingsRemoteIcon from '@material-ui/icons/SettingsRemote';
import StorageIcon from '@material-ui/icons/Storage';
import { PROJECT_PATH } from '../api';

class ProjectMenu extends Component<RouteComponentProps> {

  render() {
    const path = this.props.match.url;
    return (
      <List>
        <ListItem to={`/${PROJECT_PATH}/demo/information`} selected={path.startsWith(`/${PROJECT_PATH}/demo/`)} button component={Link}>
          <ListItemIcon>
            <SettingsRemoteIcon />
          </ListItemIcon>
          <ListItemText primary="DataLab" />
        </ListItem>
        <ListItem to={`/${PROJECT_PATH}/history/explorer`} selected={path.startsWith(`/${PROJECT_PATH}/history/`)} button component={Link}>
          <ListItemIcon>
            <StorageIcon />
          </ListItemIcon>
          <ListItemText primary="Historical Data" />
        </ListItem>
      </List>
    )
  }

}

export default withRouter(ProjectMenu);
