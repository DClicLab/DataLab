
import React, { FC } from 'react';
import { Navigate, Route, Routes } from 'react-router-dom';

import { Tab } from '@mui/material';

import { RouterTabs, useRouterTab, useLayoutTitle } from '../components';

import DemoInformation from './DemoInformation';
import ManageSensorsController from './ManageSensorsController';
import ManageSensorsForm from './ManageSensorsForm';
// import LightStateRestController from './LightStateRestController';
// import LightStateWebSocketController from './LightStateWebSocketController';
// import LightMqttSettingsController from './LightMqttSettingsController';
// import FileList from './FilesController';

const DemoProject: FC = () => {
  useLayoutTitle("Demo Project");
  const { routerTab } = useRouterTab();

  return (
    <>
      <RouterTabs value={routerTab}>
        <Tab value="information" label="Live View" />
        <Tab value="sensors" label="Sensors" />
        {/* <Tab value="rest" label="REST Example" /> */}
        {/* <Tab value="socket" label="WebSocket Example" /> */}
        {/* <Tab value="mqtt" label="MQTT Settings" /> */}
      </RouterTabs>
      <Routes>
        <Route path="information" element={<DemoInformation />} />
        <Route path="sensors" element={<ManageSensorsController />} />
        <Route path="/*" element={<Navigate replace to="information" />} />
      </Routes>
    </>
  );
};

export default DemoProject;
