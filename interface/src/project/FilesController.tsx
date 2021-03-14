import React, { Component, } from 'react';
import {  WithSnackbarProps } from 'notistack';
import { RouteComponentProps } from 'react-router-dom'

import { withAuthenticatedContext, AuthenticatedContextProps } from '../authentication';

import { ENDPOINT_ROOT } from '../api';
import { ErrorButton, restController, RestControllerProps, RestFormLoader, SectionContent, } from '../components';
import { Box, Button, createStyles, Dialog, DialogActions, DialogContent, DialogTitle, Table, TableBody, TableCell, TableHead, TableRow, Theme, Typography, withStyles, WithStyles } from '@material-ui/core';
import DeleteIcon from '@material-ui/icons/Delete';
import GetApp from '@material-ui/icons/GetApp';
import IconButton from '@material-ui/core/IconButton';
import DeleteSweepIcon from '@material-ui/icons/DeleteSweep';
import { DataFile, FilesState } from './types';
import { format } from 'date-fns';
import { toInteger } from 'lodash';
import { redirectingAuthorizedFetch } from '../authentication';
export const FILES_ENDPOINT = ENDPOINT_ROOT + "files";

const mystyles = (theme: Theme) => createStyles(
  {
    "container": {width:"100%","background-color":"#ddd",display:"flex"},
    "storagebar": {paddingLeft:"5px",paddingTop:"10px",paddingBottom:"10px", backgroundColor: theme.palette.primary.main, color: theme.palette.getContrastText(theme.palette.primary.main)},
    "www": {paddingLeft:"5px",paddingTop:"10px",paddingBottom:"10px", backgroundColor: theme.palette.warning.main, color: theme.palette.getContrastText(theme.palette.primary.main)},
    "free": {paddingLeft:"5px",paddingTop:"10px",paddingBottom:"10px", backgroundColor: theme.palette.secondary.main, color: theme.palette.getContrastText(theme.palette.primary.main)}
  });

type FileListProps = WithSnackbarProps & RestControllerProps<FilesState>;

interface IState {
  graphFile?: DataFile
  confirmDeleteAll: boolean;
  processing: boolean;
}


class FileList extends Component<FileListProps & RouteComponentProps & AuthenticatedContextProps & WithStyles<typeof mystyles>, IState > {
  
  state: IState={
    confirmDeleteAll: false,
    processing: false
  }

  componentDidMount() {
    this.props.loadData();
  }

  deleteFile(this: this, file: DataFile) {
    var formData = new FormData(); // Currently empty
    formData.append("ts", String(file.start));

    fetch(FILES_ENDPOINT + "/delete", { body: formData, method: "POST" }).then(() => {
      this.props.enqueueSnackbar("Successfully deleted: " + file.name, {
        variant: 'success',
      })
    }).catch((error) => {
      this.props.enqueueSnackbar("Error deleting: " + file.name + "\n " + error, {
        variant: 'error',
      })
    });
    this.props.loadData();
    this.forceUpdate();
  };

  renderDeleteAllDialog() {
    return (
      <Dialog
        open={this.state.confirmDeleteAll}
        onClose={this.onDeleteAllRejected}
      >
        <DialogTitle>Confirm Delete All</DialogTitle>
        <DialogContent dividers>
          Are you sure you want to delete all datafile on the device?<br/>The device will restart.
        </DialogContent>
        <DialogActions>
          <Button variant="contained" onClick={this.onDeleteAllRejected} color="secondary">
            Cancel
          </Button>
          <ErrorButton startIcon={<DeleteSweepIcon />} variant="contained" onClick={this.onDeleteAllConfirmed} disabled={this.state.processing} autoFocus>
            Delete all
          </ErrorButton>
        </DialogActions>
      </Dialog>
    )
  }

  onDeleteAll = () => {
    this.setState({ confirmDeleteAll: true });
  }

  onDeleteAllRejected = () => {
    this.setState({ confirmDeleteAll: false });
  }

  onDeleteAllConfirmed = () => {
    this.setState({ processing: true });
    redirectingAuthorizedFetch(ENDPOINT_ROOT+"deleteall", { method: 'POST' })
      .then(response => {
        if (response.status === 200) {
          this.props.enqueueSnackbar("Delete all in progress. Refreshing page in 5 sec.", { variant: 'error' });
          this.setState({ processing: false, confirmDeleteAll: false });
          setTimeout(() => {
            window.location.reload(false);
          }, 5000);
        } else {
          throw Error("Invalid status code: " + response.status);
        }
      })
      .catch(error => {
        this.props.enqueueSnackbar(error.message || "Problem factory resetting device", { variant: 'error' });
        this.setState({ processing: false });
      });
  }

  

  render() {
    let wwwsize = 430000
    
    return (
      <RestFormLoader
            {...this.props}
            render={props => (
              <>
              <SectionContent title="Storage use">
              <div className={this.props.classes.container}>
                  <div className={this.props.classes.www} style={{width:wwwsize / this.props.data!.space!.total*100 + "%"}}>System<br/>{toInteger(wwwsize/1000)}KB</div> 
                  <div className={this.props.classes.storagebar} style={{width:(this.props.data!.space!.used - wwwsize) / this.props.data!.space!.total*100 + "%"}}>Data<br/>{toInteger((this.props.data!.space!.used-wwwsize)/1000)}KB</div> 
                  <div className={this.props.classes.free} style={{width:100-this.props.data!.space!.used / this.props.data!.space!.total*100 + "%"}}>Free space<br/>{toInteger((this.props.data!.space!.total - this.props.data!.space!.used)/1000)}KB</div> 
              </div>
              </SectionContent>
              <SectionContent title='Data file list' titleGutter>
              <Table>
                <TableHead>
                  <TableRow>
                    <TableCell>File</TableCell>
                    <TableCell>Start</TableCell>
                    <TableCell>End</TableCell>
                    <TableCell >Nb of values</TableCell>
                    <TableCell colSpan={8}>Size</TableCell>
                  </TableRow>
                </TableHead>
                <TableBody>
                  {this.props.data!.files!.map(file => (
                    <TableRow key={file.name}><TableCell>{file.name}</TableCell>
                      <TableCell>{tsToTime(Number(file.start))}</TableCell>
                      <TableCell>{file.diff == 0 ? "Unknown" : tsToTime(Number(file.end))}</TableCell>
                      <TableCell>{file.nval}</TableCell>
                      <TableCell>{toInteger(file.nval*7/1000)}KB</TableCell>
                      <TableCell><IconButton disabled={!this.props.authenticatedContext.me.admin} size="small" aria-label="Delete" onClick={() => this.deleteFile(file)}><DeleteIcon /></IconButton></TableCell>
                      <TableCell><a href={ENDPOINT_ROOT + "getjson"} download target="_blank"><IconButton size="small" aria-label="Download"><GetApp /></IconButton></a></TableCell>
                    </TableRow>
                  ))
                  }
                </TableBody>
              </Table>
              <Box flexWrap="none" alignItems="right"  padding={1} whiteSpace="nowrap">
              {this.props.authenticatedContext.me.admin?
              <ErrorButton startIcon={<DeleteSweepIcon />} variant="contained" onClick={this.onDeleteAll}>
                Delete all
              </ErrorButton>
              :""}
              </Box>
              </SectionContent>
              {this.renderDeleteAllDialog()}
              </>
            )}
            />
            
            )
  }
}

export default withAuthenticatedContext(withStyles(mystyles)(restController(FILES_ENDPOINT, FileList)));

function tsToTime(ts: number) {
  return format(ts * 1000, "yyyy-MM-dd HH:mm");
}

function useStyles() {
  throw new Error('Function not implemented.');
}

