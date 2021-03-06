import React, { Component, PureComponent, useEffect, useState,SVGProps } from 'react';
import { withSnackbar, WithSnackbarProps } from 'notistack';
import { RouteComponentProps } from 'react-router-dom'

import { ENDPOINT_ROOT } from '../api';
import { restController, RestControllerProps, RestFormLoader, SectionContent, } from '../components';
import { Table, TableBody, TableCell, TableHead, TableRow } from '@material-ui/core';
import DeleteIcon from '@material-ui/icons/Delete';
import TimelineIcon from '@material-ui/icons/TimelineOutlined';
import GetApp from '@material-ui/icons/GetApp';
import IconButton from '@material-ui/core/IconButton';
import 'react-day-picker/style.css';
import { DataFile, FilesState } from './types';
import { LineChart, Line, CartesianGrid, XAxis, YAxis, ResponsiveContainer, Brush, Tooltip} from 'recharts'
import { DateRange, DayPicker, SelectRangeEventHandler, DateAfter, DateBefore, DateBeforeAfter } from 'react-day-picker';
import { format } from 'date-fns';
import { MenuAppBar } from '../components';
import { toInteger } from 'lodash';
export const FILES_ENDPOINT = ENDPOINT_ROOT + "files";


type FileListProps = WithSnackbarProps & RestControllerProps<FilesState>;

export interface IState {
  graphFile: DataFile
}


class FileList extends Component<FileListProps & RouteComponentProps, IState & DataViewProps & ICalendarProps> {

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


  render() {
    return (
      <MenuAppBar sectionTitle="DataLab">
        <SectionContent title='Data file list' titleGutter>
          <RestFormLoader
            {...this.props}
            render={props => (
              <Table>
                <TableHead>
                  <TableRow>
                    <TableCell>File</TableCell>
                    <TableCell>Start</TableCell>
                    <TableCell>End</TableCell>
                    <TableCell colSpan={8}>Nb of values</TableCell>

                  </TableRow>
                </TableHead>
                <TableBody>
                  {this.props.data!.map(file => (
                    <TableRow key={file.name}><TableCell>{file.name}</TableCell>
                      <TableCell>{tsToTime(Number(file.start))}</TableCell>
                      <TableCell>{file.diff == 0 ? "Unknown" : tsToTime(Number(file.end))}</TableCell>
                      <TableCell>{file.nval}</TableCell>
                      <TableCell><IconButton size="small" aria-label="Delete" onClick={() => this.deleteFile(file)}><DeleteIcon /></IconButton></TableCell>
                      <TableCell><a href={ENDPOINT_ROOT + "getjson"} download target="_blank"><IconButton size="small" aria-label="Download"><GetApp /></IconButton></a></TableCell>

                    </TableRow>
                  ))
                  }
                </TableBody>
              </Table>
            )}
          />
          <Calendar files={this.props.data!} />

        </SectionContent>
      </MenuAppBar>
    )
  }
}

export default restController(FILES_ENDPOINT, FileList);

function tsToTime(ts: number) {
  return format(ts * 1000, "dd/MM/yyyy HH:mm");
}

export interface DataViewProps {
  graphData: any[],
  graphIndex: IIndex[],
  downloadCall?:(start: number, end: number, sensorName: string) => void

}
interface BrushStartEndIndex {
  startIndex?: number;
  endIndex?: number;
}

class DataViewForm extends Component<DataViewProps> {
  colors = ['#0045ff', '#6378fb', '#8ca9f5', '#b3d9ed', '#ffffe0', '#eec050', '#da854e', '#ba4d3b', '#8a2111'];
  componentDidUpdate() {
    console.log(this.props.graphData, this.props.graphIndex)
  }

  RenderTick = (tick: number) => ((new Date(tick * 1000).toLocaleDateString("fr-FR")) + " " + (new Date(tick * 1000).toLocaleTimeString("fr-FR")));
  
  brushIndex:BrushStartEndIndex={};
  
  BrushChange =  ((newIndex: BrushStartEndIndex)=>{
    this.brushIndex=newIndex
  });

  prepareDownload=(sensorName:string)=> {
    var tss=this.props.graphData[this.brushIndex.startIndex || 0].ts;
    var tse=this.props.graphData[this.brushIndex.endIndex || -1].ts;
    this.props.downloadCall!(tss,tse,sensorName);
  }

  render() {
    if (this.props.graphData && this.props.graphIndex) {
      return (
        this.props.graphIndex.map((keyindex, index) => (
          <SectionContent title={keyindex.name} titleGutter key={index}>
            <ResponsiveContainer width="95%" height={400}>
              <LineChart height={300} data={this.props.graphData} syncId="datalab" >
                {(index == 0) ?
// @ts-ignore
                  <Brush  dataKey="ts" tickFormatter={this.RenderTick} onChange={this.BrushChange} />
                  : ""}
                <XAxis dataKey="ts" tickFormatter={this.RenderTick} />
                <YAxis />
                <Tooltip labelFormatter={this.RenderTick} />
                <CartesianGrid stroke="#eee" strokeDasharray="5 5" />
                <Line type="monotone" connectNulls={true} dataKey={keyindex.name} stroke={this.colors[index]} />
                {/* <Line type="monotone" dataKey="pv" stroke="#82ca9d" /> */}
              </LineChart>
            </ResponsiveContainer>
            <IconButton size="small" aria-label="Download" onClick={() => this.prepareDownload!(keyindex.name)}><GetApp /></IconButton>
          </SectionContent>
        ))
      )
    }
    return (<div>Nothing selected</div>)
  }
}


export interface DataRetriverProps {
  range: DateRange
};


interface ICalendarProps {
  files: FilesState
}

interface IIndex
{ name: string, index: number };


function Calendar(props: ICalendarProps) {
  const defaultSelected: DateRange = {
    from: new Date(),
    to: new Date()
  };
  
  const [sampling, setSampling] = useState(0);

  const [range, setRange] = useState(defaultSelected);
  const handleSelect: SelectRangeEventHandler = (range) => {
    setRange(range!);
  };
  
  var indexes: IIndex[];
  const [datapoints, setDatapoints] = useState<DataPoint[]>([]);
  // var datapoints: DataPoint[]=[];
  
  var displayGraph = (range: DateRange) => {
    console.log("retreiving for ", range)
    var p: Promise<Response>[] = [];
    var p1: Promise<void>[] = [];
    var files: FilesState;
    range.from.setHours(0,0,0,0);

    files = props.files.filter((file) => !(file.end*1000 < +range.from || file.start*1000 > +range.to!+3600000*24));

    console.log("files: ",files);
    
    //get indexes
    fetch(ENDPOINT_ROOT + 'indexes')
      .then(response => {
        return response.json() as Promise<IIndex[]>;
      })
      .then(indexesJ => {
        indexes = indexesJ.sort((a,b)=>a.index-b.index)
        console.log(indexes);
        files.forEach(file => {
          p.push(fetch(ENDPOINT_ROOT + "../raw/" + file.start))
        });
        var ldatapoints:DataPoint[] = []
        Promise.all(p).then(responses => {
          console.log("all files fetched");
          responses.forEach(response => {
            p1.push(response.arrayBuffer().then(buffer => {
              console.log("extractdp",response,response.url.split('/')[-1])
              ldatapoints = ldatapoints.concat(extractDP(buffer,indexes,Number(response.url.split('/').pop())));
              console.log("file processed.")
            }));
          });
          Promise.all(p1).then(res=>{
            console.log("done",ldatapoints,res)
            setDatapoints(ldatapoints)
            var sampling=toInteger(ldatapoints.length/250)
            setSampling(sampling);

            let merged=getMergedDP(ldatapoints, sampling);
            
            // Merge datapoints by ts
            setDataToGraph({graphData:merged,graphIndex:indexes})       
        })
        });
      })
      .catch(function (e) {
        console.log(e);
      });
  };
  
  const [disabledDays, setDisabledDays] = useState<(DateBefore | DateAfter)[]>([{ after: new Date() }]);
  const [dataToGraph, setDataToGraph] = useState<DataViewProps>();


  useEffect(() => {
    if (props.files) {
      var datemin = props.files?.reduce((prev, curr) => ((prev.start != 0) && prev.start < curr.start) ? prev : curr).start
      var datemax = props.files?.reduce((prev, curr) => prev.end > curr.end ? prev : curr).end
      var start = new Date(datemin * 1000)
      var end = new Date(datemax * 1000)
      console.log("Got some dates in files:", start, end)
      setDisabledDays([{ before: start }, { after: end }]);
    }
  }, [props.files]);

  useEffect(() => {
    console.log("graph ready",dataToGraph)
  }, [dataToGraph]);

  var downloadCall=(start:number,end:number,sensorName:string)=>{
    downloadCSV(sensorName+".csv",datapoints,start,end,sensorName);
  }
  return (
    <>
      <DayPicker
        mode="range"
        disabled={disabledDays}
        numberOfMonths={2}
        defaultSelected={defaultSelected}
        onSelectRange={handleSelect}
        showOutsideDays={true}
      />
      <p>
        You picked from {range && range.from && range.from.toLocaleDateString()}{' '}
        to {range && range.to && range.to.toLocaleDateString()}
      </p>
      <p>Sampling is {sampling?sampling: "null"}</p>
      <IconButton size="small" aria-label="Graph" onClick={() => displayGraph(range)}><TimelineIcon /></IconButton>

      {dataToGraph? 
      <DataViewForm graphData={dataToGraph.graphData} graphIndex={dataToGraph.graphIndex} downloadCall={downloadCall}/>
        : "No data to graph"
      }
    </>
  );
}

interface DataPoint {
  id: number,
  ts?: number,
  val: number,
  index:string
  tsdiff: number
}

// function getMergedDP(datapoints: DataPoint[],  sampling: number, sensorNames:string[]=[], onlySensor:string=""):any[]{
//   let merged = [];
//   if (sampling<1) sampling=1;
//   loop1:
//   for (let index = 0; index < datapoints.length;) {
//     var point: any = {};
//     do {
//       const dp = datapoints[index];
//       if (onlySensor!="" && dp.index!=onlySensor) { index++; continue loop1} ;//skip this point
//       point.ts = dp.ts;
//       point[dp.index] = dp.val;
//       if (!sensorNames.includes(dp.index)) sensorNames.push(dp.index);
//       index++;
//     } while (index < datapoints.length && datapoints[index].ts == point.ts);
//     merged.push(point);
//     if (index + sampling < datapoints.length)
//       index += sampling;
//   }
//   merged=merged.sort((a,b)=>a.ts-b.ts)
//   return merged;
// }
function getMergedDP(datapoints: DataPoint[],  sampling: number, sensorNames:string[]=[], onlySensor:string=""):any[]{
  let merged = [];
  if (sampling<1) sampling=1;
  loop1:
  for (let index = 0; index < datapoints.length;) {
    var point: any = {};
    do {
      const dp = datapoints[index];
      if (onlySensor!="" && dp.index!=onlySensor) { index++; continue loop1} ;//skip this point
      point.ts = dp.ts;
      point[dp.index] = dp.val;
      if (!sensorNames.includes(dp.index)) sensorNames.push(dp.index);
      index++;
    } while (index < datapoints.length && datapoints[index].ts == point.ts);
    merged.push(point);
    if (index + sampling < datapoints.length)
      index += sampling;
  }
  merged=merged.sort((a,b)=>a.ts-b.ts)
  return merged;
}

function extractDP(buffer: ArrayBuffer,indexes: IIndex[], tsstart:number): DataPoint[] {
  var ret: DataPoint[] = [];
  console.log("Bytelength: ",buffer)
  var i=0;
  for (let offset = 0; offset *7 < buffer.byteLength; offset ++) {
    i++
    ret.push(ReadDataPoint(buffer.slice(7 * offset, 7 * offset + 7),indexes, tsstart));
  }
  console.log("points nub: "+i)
  return ret;
}

function ReadDataPoint(buffer: ArrayBuffer,indexes:IIndex[],tsstart:number): DataPoint {
  var v = new Uint8Array(buffer);
  var id = v[0] & 5;
  var tsdiff = v[2] << 11 | v[1] << 3 | (v[0] >>> 5)
  if (tsdiff<0){
    console.log("err",v,tsdiff)
  }
  var float = new DataView(v.slice(3).reverse().buffer).getFloat32(0);
  return { id: id, tsdiff: tsdiff,ts:(tsdiff+tsstart), val: float, index:indexes[id].name};
}


function downloadCSV(filename:string, datapoints:DataPoint[], start:number=0, end:number=0,onlySensor:string="") {
  var text:string=""
  var selectedPoints:DataPoint[]=[]
  console.log("In download csv",filename,start,end,onlySensor);
  
  for (let index = 0; index < datapoints.length; index++) {
    const point = datapoints[index];
    if (point.ts!>start && (end==0 || point.ts!<end)){
        selectedPoints.push(point)
    }
  }
  var sensorNames:string[]=[]
  var mergedPoints = getMergedDP(selectedPoints,0,sensorNames,onlySensor); 
  mergedPoints.forEach(point => {
    var line=format(point.ts*1000, 'yyyy-MM-dd HH:mm:ss')+","
    sensorNames.forEach(sensorName => {
      line+=(point.hasOwnProperty(sensorName)?point[sensorName]:'')+","
    });
    text+=line.slice(0,-1)+"\n"
  });
  console.log("final csv:",text)
  var element = document.createElement('a');
  element.setAttribute('href', 'data:text/csv;charset=utf-8,' + encodeURIComponent(text));
  element.setAttribute('download', filename);
  element.style.display = 'none';
  document.body.appendChild(element);
  element.click();
  document.body.removeChild(element);
}