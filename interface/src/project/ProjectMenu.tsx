import { FC } from 'react';

import { List } from '@mui/material';
import SettingsRemoteIcon from '@mui/icons-material/SettingsRemote';

import { PROJECT_PATH } from '../api/env';
import LayoutMenuItem from '../components/layout/LayoutMenuItem';

const ProjectMenu: FC = () => (
  <List>
    <LayoutMenuItem icon={SettingsRemoteIcon} label="Demo Project" to={`/${PROJECT_PATH}/demo`} />
  </List>
);

//   render() {
//     const path = this.props.match.url;
//     return (
//       <List>
//         <ListItem to={`/${PROJECT_PATH}/demo/information`} selected={path.startsWith(`/${PROJECT_PATH}/demo/`)} button component={Link}>
//           <ListItemIcon>
//             <SettingsRemoteIcon />
//           </ListItemIcon>
//           <ListItemText primary="DataLab" />
//         </ListItem>
//         <ListItem to={`/${PROJECT_PATH}/history/explorer`} selected={path.startsWith(`/${PROJECT_PATH}/history/`)} button component={Link}>
//           <ListItemIcon>
//             <StorageIcon />
//           </ListItemIcon>
//           <ListItemText primary="Historical Data" />
//         </ListItem>
//       </List>
//     )
//   }

// }

// export default withRouter(ProjectMenu);
export default ProjectMenu;
