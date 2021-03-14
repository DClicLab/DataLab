import React, { Component, useEffect, useState } from 'react';
import { WithSnackbarProps } from 'notistack';
import { RouteComponentProps } from 'react-router-dom'

import { ENDPOINT_ROOT } from '../api';
import { restController, RestControllerProps, RestFormLoader, SectionContent, } from '../components';
import { Box, Button, makeStyles, Typography, WithStyles } from '@material-ui/core';
import TimelineIcon from '@material-ui/icons/TimelineOutlined';
import GetApp from '@material-ui/icons/GetApp';
import 'react-day-picker/style.css';
import { DataFile, FilesState } from './types';
import { LineChart, Line, CartesianGrid, XAxis, YAxis, ResponsiveContainer, Brush, Tooltip} from 'recharts'
import { DateRange, DayPicker, SelectRangeEventHandler, DateAfter, DateBefore } from 'react-day-picker';
import { format } from 'date-fns';
import { toInteger } from 'lodash';
export const FILES_ENDPOINT = ENDPOINT_ROOT + "files";

const useStyles = makeStyles((theme) => ({
  button: {
    margin: theme.spacing(1),
  },
}));

type FileListProps = WithSnackbarProps & RestControllerProps<FilesState>;

export interface IState {
  graphFile: DataFile
}


class DataExplorer extends Component<FileListProps & RouteComponentProps, IState & DataViewProps & ICalendarProps> {

  componentDidMount() {
    this.props.loadData();
  }


  render() {
    return (
        <SectionContent title='Data Explorer' titleGutter>
          
          <RestFormLoader
                      {...this.props}
                      render={props => (
          <Calendar fsinfo={this.props.data!} />
                      )}/>
        
        </SectionContent>
    )
  }
}

export default restController(FILES_ENDPOINT, DataExplorer);

export interface DataViewProps {
  graphData: any[],
  graphIndex: IIndex[],
  downloadCall?:(start: number, end: number, sensorName: string) => void
  sensorInfos: Map<string, {
    sampling: number;
    index: number;
}>
}
interface BrushStartEndIndex {
  startIndex?: number;
  endIndex?: number;
}

export interface DataRetriverProps {
  range: DateRange
};


interface ICalendarProps {
  fsinfo: FilesState
}

interface IIndex
{ name: string, index: number };


function Calendar(props: ICalendarProps) {
  const defaultSelected: DateRange = {
    from: new Date(),
    to: new Date()
  };
  
  const [sensorInfo, setSensorInfos] = useState<Map<string,{sampling:number, index:number}>>(new Map());

  const [range, setRange] = useState(defaultSelected);
  const handleSelect: SelectRangeEventHandler = (range) => {
    setRange(range!);
  };
  
  var indexes: IIndex[];
  const [datapoints, setDatapoints] = useState<DataPoint[]>([]);
  
  var displayGraph = (range: DateRange) => {
    console.log("retreiving for ", range)
    var p: Promise<Response>[] = [];
    var p1: Promise<void>[] = [];
    var fsinfo: FilesState={};
    range.from.setHours(0,0,0,0);

    fsinfo.files = props.fsinfo.files!.filter((file) => !(file.end*1000 < +range.from || file.start*1000 > +range.to!+3600000*24));

    fetch(ENDPOINT_ROOT + 'indexes')
      .then(response => {
        return response.json() as Promise<IIndex[]>;
      })
      .then(indexesJ => {
        indexes = indexesJ.sort((a,b)=>a.index-b.index)
        console.log(indexes);
        fsinfo.files!.forEach(file => {
          p.push(fetch(ENDPOINT_ROOT + "../raw/" + file.start))
        });
        var ldatapoints:DataPoint[] = []
        Promise.all(p).then(responses => {
          console.log("all files fetched");
          responses.forEach(response => {
            p1.push(response.arrayBuffer().then(buffer => {
              console.log("extractdp",response,response.url.split('/')[-1])
              ldatapoints = ldatapoints.concat(extractDP(buffer,indexes,Number(response.url.split('/').pop()),range));
              console.log("file processed.")
            }));
          });
          Promise.all(p1).then(res=>{
            console.log("done",ldatapoints,res)
            setDatapoints(ldatapoints)
                       
            const [points, sensorInfos]=getMergedDP(ldatapoints,500);
            setSensorInfos(sensorInfos);
            // Merge datapoints by ts
            setDataToGraph({graphData:points,graphIndex:indexes,sensorInfos:sensorInfos})       
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
    if (props.fsinfo.files) {
      var datemin = props.fsinfo.files?.reduce((prev, curr) => ((prev.start != 0) && prev.start < curr.start) ? prev : curr).start
      var datemax = props.fsinfo.files?.reduce((prev, curr) => prev.end > curr.end ? prev : curr).end
      var start = new Date(datemin * 1000)
      var end = new Date(datemax * 1000)
      console.log("Got some dates in files:", start, end)
      setDisabledDays([{ before: start }, { after: end }]);
    }
  }, [props.fsinfo.files]);

  useEffect(() => {
    console.log("graph ready",dataToGraph)
  }, [dataToGraph]);


  var downloadCall=(start:number,end:number,sensorName:string)=>{
    downloadCSV(sensorName+".csv",datapoints,start,end,sensorName);
  }
  const classes = useStyles();
  return (
    <>
      <Box flexDirection="column" display="flex" alignItems="center">
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

      {/* <p>Sampling is {Array.from(sensorInfo.values()).map((infos, index) => {
          return <p>{"For index:"+ index + " Val:" + infos.index +" Sampling: " + infos.sampling}</p>
      })}
      </p> */}

      {/* {Array.from(sensorInfo.keys()).map((entry)=>{console.log(entry);return <p>{entry}</p>}) } */}
      <Button className={classes.button} variant="contained" onClick={() => displayGraph(range)} color="primary" startIcon={<TimelineIcon/>} disableElevation>
      Generate graphs
</Button>
      </Box>
      {dataToGraph? 
      <DataViewForm classes={classes} graphData={dataToGraph.graphData} graphIndex={dataToGraph.graphIndex} downloadCall={downloadCall} sensorInfos={sensorInfo}/>
        : ""
      }
    </>
  );
}

class DataViewForm extends Component<DataViewProps & WithStyles> {
  colors = ['#0045ff', '#6378fb', '#8ca9f5', '#b3d9ed', '#ffffe0', '#eec050', '#da854e', '#ba4d3b', '#8a2111'];
  componentDidUpdate() {
    console.log(this.props.graphData, this.props.graphIndex)
  }

  RenderTick = (tick: number) => ((new Date(tick * 1000).toLocaleDateString("fr-FR")) + " " + (new Date(tick * 1000).toLocaleTimeString("fr-FR")));
  
  brushIndex:BrushStartEndIndex={};
  
  BrushChange =  ((newIndex: BrushStartEndIndex)=>{
    this.brushIndex=newIndex
  });

  prepareRangeDownload=(sensorName:string)=> {
    var tss=this.props.graphData[this.brushIndex.startIndex || 0].ts;
    var tse=this.props.graphData[this.brushIndex.endIndex || -2].ts;
    this.props.downloadCall!(tss,tse,sensorName);
  }
  prepareDownload=(sensorName:string)=> {
    var tss=this.props.graphData[0].ts;
    // for (var i = this.props.graphData.length-1; i >0 && this.props.graphData[i].ts== undefined; i--) {
    // }
    // var tse=this.props.graphData[i].ts;
    var tse=this.props.graphData.slice(-1)[0].ts
    this.props.downloadCall!(tss,tse,sensorName);
  }

  render() {
    if (this.props.graphData && this.props.graphIndex) {
      return (
        
        this.props.graphIndex.map((keyindex, index) => (
          this.props.sensorInfos.get(keyindex.name)?.index ?
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
                <Line type="linear" connectNulls={true} dataKey={keyindex.name} />
                {/* <Line type="monotone" dataKey="pv" stroke="#82ca9d" /> */}
              </LineChart>
                </ResponsiveContainer>
              <Typography variant="h6" >
          {
          this.props.sensorInfos.get(keyindex.name)?.index+" values in datafile. Sampling rate: 1 point in graph for "+this.props.sensorInfos.get(keyindex.name)?.sampling+" points in datafile."
          }
        </Typography>

        <Button className={this.props.classes.button} variant="contained" onClick={() => this.prepareRangeDownload!(keyindex.name)} color="primary" startIcon={<GetApp/>} disableElevation>
      Download displayed range data
</Button>
        <Button className={this.props.classes.button} variant="contained" onClick={() => this.prepareDownload!(keyindex.name)} color="primary" startIcon={<GetApp/>} disableElevation>
      Download all graph data
</Button>
          </SectionContent>
          : ""
        ))
      )
    }
    return (<div>Nothing selected</div>)
  }
}

interface DataPoint {
  id: number,
  ts?: number,
  val: number,
  index:string
  tsdiff: number
}


function getMergedDP(datapoints: DataPoint[], maxValPerSensor=0):[any[] , Map<string,{sampling:number, index:number}>]{
  let sensorInfo:Map<string,number>=new Map()
  let sampling:Map<string,{sampling:number, index:number}>=new Map();
  let merged = [];
  //points are already filtering with start . end
  //count values foreach sensor
  for (let i = 0; i < datapoints.length; i++) {
    const dp = datapoints[i];
    sensorInfo.set(dp.index,(sensorInfo.get(dp.index)||0)+1)
  }
  sensorInfo.forEach((nval,name) => {
    sampling.set(name,{sampling:((nval<maxValPerSensor || maxValPerSensor==0)?1:toInteger(nval/maxValPerSensor)),index:0});
  });

  loop1:
  for (let index = 0; index < datapoints.length;) {
    var point: any = {};
    do {
      const dp = datapoints[index];
      let samp = sampling.get(dp.index)!;
      if ((samp.index++ % samp.sampling !=0)) { if (point.ts) merged.push(point); index++; continue loop1} ;//skip this point
      point.ts = dp.ts;
      point[dp.index] = dp.val;
      index++;
    } while (index < datapoints.length && datapoints[index].ts == point.ts);
    merged.push(point);
    // if (index < datapoints.length) index++;
  }
  merged=merged.sort((a,b)=>a.ts-b.ts)
  
  return [merged,sampling];
}

function extractDP(buffer: ArrayBuffer,indexes: IIndex[], tsstart:number, range:DateRange): DataPoint[] {
  var ret: DataPoint[] = [];
  var i=0;
  for (let offset = 0; offset *7 < buffer.byteLength; offset ++) {
    i++
    var point=ReadDataPoint(buffer.slice(7 * offset, 7 * offset + 7),indexes, tsstart);
    if (point.ts! *1000>+range.from && point.ts!*1000<+range.to!)
      ret.push(point);
  }
  return ret;
}

function ReadDataPoint(buffer: ArrayBuffer,indexes:IIndex[],tsstart:number): DataPoint {
  var v = new Uint8Array(buffer);
  var id = v[0] & 5;
  var tsdiff = (v[2] << 11) | (v[1] << 3) | (v[0] >>> 5)
  var float = new DataView(v.slice(3).reverse().buffer).getFloat32(0);
  return { id: id, tsdiff: tsdiff,ts:(tsdiff+tsstart), val: float, index:indexes[id].name};
}


function downloadCSV(filename:string, datapoints:DataPoint[], start:number=0, end:number=0,onlySensor:string="") {
  var text:string=""
  var selectedPoints:DataPoint[]=[]
  for (let index = 0; index < datapoints.length; index++) {
    const point = datapoints[index];
    if (point.ts!>start && (end==0 || point.ts!<end) && (onlySensor=="" || onlySensor==point.index)){
        selectedPoints.push(point)
    }
  }
  var sensorNames:string[]=[]
  const [mergedPoints,sensorInfo] = getMergedDP(selectedPoints);
  sensorInfo.forEach((info,name) => {
    sensorNames.push(name)
  });
  text="time,"+sensorNames.join(',')+"\n"

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