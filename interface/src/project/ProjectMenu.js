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
        <ListItem to={`/${PROJECT_PATH}/data/`} selected={path.startsWith(`/${PROJECT_PATH}/data/`)} button component={Link}>
        <ListItemIcon>
            <SettingsRemoteIcon />
          </ListItemIcon>
          <ListItemText primary="DataLab 2.0" />
        </ListItem>
      </List>
    )
  }

}

export default withRouter(ProjectMenu);
