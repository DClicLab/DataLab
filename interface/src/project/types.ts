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

export interface FilesState extends Array<DataFile>{};

export interface LightMqttSettings {
  unique_id: string;
  name: string;
  mqtt_path: string;
}

// export interface Sensor {
//   name: string;
//   driver: Driver;
//   enabled: boolean;

// export class Driver {
//   name: string = "";
//   params?: Param[];
// }



export interface Driver {
  name: string;
  config?:  unknown;
}
// }
export class Sensor {
  name: string = "" ;
  // driver: Driver = new Driver();
  driver: Driver = {"name":""};
  enabled: boolean = true;
  interval: number = 0;
}

// export interface Param {
//   name: string;
//   type: "str" | "int";
//   default: string | number;
//   value: string | number;
// }

export interface SensorsSettings {
  sensors: Sensor[];
  drivers: Driver[];
}