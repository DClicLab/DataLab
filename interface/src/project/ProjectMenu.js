import React, { Component } from 'react';
import { Link, withRouter } from 'react-router-dom';

import { PROJECT_PATH } from '../constants/Env';

import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import ListItemIcon from '@material-ui/core/ListItemIcon';
import ListItemText from '@material-ui/core/ListItemText';
import SettingsRemoteIcon from '@material-ui/icons/SettingsRemote';

class ProjectMenu extends Component {

  render() {
    const path = this.props.match.url;
    return (
      <List>
        <ListItem to={`/${PROJECT_PATH}/demo/`} selected={path.startsWith(`/${PROJECT_PATH}/demo/`)} button component={Link}>
        <ListItemIcon>
            <SettingsRemoteIcon />
          </ListItemIcon>
          <ListItemText primary="CaptoSensors" />
        </ListItem>
      </List>
    )
  }

}

export default withRouter(ProjectMenu);
