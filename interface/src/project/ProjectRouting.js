import React, { Component } from 'react';
import { Redirect, Switch } from 'react-router';

import { PROJECT_PATH } from '../constants/Env';
import AuthenticatedRoute from '../authentication/AuthenticatedRoute';
import DataProject from './DataProject';

class ProjectRouting extends Component {

  render() {
    return (
      <Switch>
        {
          /*
          * Add your project page routing below.
          */
        }
        <AuthenticatedRoute exact path={`/${PROJECT_PATH}/data/*`} component={DataProject} />
        {
          /*
          * The redirect below caters for the default project route and redirecting invalid paths.
          * The "to" property must match one of the routes above for this to work correctly.
          */
        }
        <Redirect to={`/${PROJECT_PATH}/data/`} />
      </Switch>
    )
  }

}

export default ProjectRouting;
