import { FC } from 'react';
import { Navigate, Routes, Route } from 'react-router-dom';

import DemoProject from './DemoProject';
import DataHistory from './Historical';

const ProjectRouting: FC = () => {
  return (
    <Routes>
      {
        // Add the default route for your project below
      }
      <Route path="/*" element={<Navigate to="demo/information" />} />
      {
        // Add your project page routes below.
      }
      <Route path="history/*" element={<DataHistory />} />
      <Route path="demo/*" element={<DemoProject />} />
    </Routes>
  );
};

export default ProjectRouting;
