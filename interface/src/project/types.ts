export interface LightState {
  led_on: boolean;
}

export interface DataFile {
  name: string;
  start: number;
  end: number;
  nval: number;
  diff: number;
}

export interface FilesState {
  files?: Array<DataFile>,
  space?: {total:number,used:number}
}

export interface LightMqttSettings {
  unique_id: string;
  name: string;
  mqtt_path: string;
}


export interface Driver {
  name: string;
  config?:  unknown;
  i2c?: number;
}
// }
export class Sensor {
  name: string = "" ;
  // driver: Driver = new Driver();
  driver: Driver = {"name":""};
  enabled: boolean = true;
  interval: number = 10;
}
export class I2CConf {
  sda: number = -1;
  scl: number = -1;
  enabled: boolean = true;
  conf: string = "Custom"
}


export interface SensorsSettings {
  sensors: Sensor[];
  drivers: Driver[];
  i2c: I2CConf;
}