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
import { withSnackbar } from 'notistack';

export const DEMO_SETTINGS_ERRORS = ENDPOINT_ROOT + "../getErrors";

return withSnackbar(
class DemoProject extends Component {

  handleTabChange = (event, path) => {
    this.props.history.push(path);
  };


  componentDidMount() {
    //    this.props.loadData();
        window.setInterval(() => {
          this.checkErrors();
        }, 2000)
      }

  checkErrors(){
    fetch(DEMO_SETTINGS_ERRORS)
      .then(res =>  {
        this.props.enqueueSnackbar(res, {
          variant: 'error',
        });
        },
        (error) => {
          // this.setState({
          //   isLoaded: true,
          //   error
          // });
        });
  }


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
)
export default DemoProject;
