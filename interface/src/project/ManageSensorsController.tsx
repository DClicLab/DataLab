import React, { Component } from 'react';

import {restController, RestControllerProps, RestFormLoader, SectionContent } from '../components';

import ManageSensorsForm from './ManageSensorsForm';
import { SensorsSettings } from './types';
import { ENDPOINT_ROOT } from '../api';
export const SENSORS_ENDPOINT = ENDPOINT_ROOT + "sensorsState";

type ManageSensorsControllerProps = RestControllerProps<SensorsSettings>;

class ManageSensorsController extends Component<ManageSensorsControllerProps> {

  componentDidMount() {
    this.props.loadData();
  }

  render() {
    return (
      <SectionContent title="Manage Sensors" titleGutter>
        <RestFormLoader
          {...this.props}
          render={formProps => <ManageSensorsForm {...formProps} />}
        />
      </SectionContent>
    )
  }

}

export default restController(SENSORS_ENDPOINT, ManageSensorsController);
