import React from 'react';
import { withSnackbar } from 'notistack';
import { redirectingAuthorizedFetch } from '../authentication/Authentication';

/*
* It is unlikely this application will grow complex enough to require redux.
*
* This HOC acts as an interface to a REST service, providing data and change
* event callbacks to the wrapped components along with a function to persist the
* changes.
*/
export const restComponent = (endpointUrl, FormComponent) => {

  return withSnackbar(
    class extends React.Component {

      constructor(props) {
        super(props);

        this.state = {
          data: null,
          fetched: false,
          errorMessage: null
        };

        this.setState = this.setState.bind(this);
        this.loadData = this.loadData.bind(this);
        this.saveData = this.saveData.bind(this);
        this.setData = this.setData.bind(this);
      }

      setData(data) {
        this.setState({
          data: data,
          fetched: true,
          errorMessage: null
        });
      }

      loadData() {
        this.setState({
          data: null,
          fetched: false,
          errorMessage: null
        });
        redirectingAuthorizedFetch(endpointUrl)
          .then(response => {
            if (response.status === 200) {
              return response.json();
            }
            throw Error("Invalid status code: " + response.status);
          })
          .then(json => { this.setState({ data: json, fetched: true }) })
          .catch(error => {
            const errorMessage = error.message || "Unknown error";
            this.props.enqueueSnackbar("Problem fetching: " + errorMessage, {
              variant: 'error',
            });
            this.setState({ data: null, fetched: true, errorMessage });
          });
      }

      saveData(e) {
        this.setState({ fetched: false });
        redirectingAuthorizedFetch(endpointUrl, {
          method: 'POST',
          body: JSON.stringify(this.state.data),
          headers: {
            'Content-Type': 'application/json'
          }
        })
          .then(response => {
            if (response.status === 200) {
              return response.json();
            }
            throw Error("Invalid status code: " + response.status);
          })
          .then(json => {
            this.props.enqueueSnackbar("Changes successfully applied.", {
              variant: 'success',
            });
            this.setState({ data: json, fetched: true });
          }).catch(error => {
            const errorMessage = error.message || "Unknown error";
            this.props.enqueueSnackbar("Problem saving: " + errorMessage, {
              variant: 'error',
            });
            this.setState({ data: null, fetched: true, errorMessage });
          });
      }

      handleValueChange = name => (event) => {
        console.log("in handleValueChange");
        console.log(this.state);
        console.log(event.target);
        console.log("data:", this.state);

        const { data } = this.state;
        data[name] = event.target.value;
        this.setState({ data });
      };


      handleSensorChange = (index, attr, configname) => (event, newValue) => {
        console.log("handleSensorChange: ", event.target, newValue);
        const { data } = this.state;
        if (configname == null) {
          data["sensors"][index][attr] = (event.target.type === "checkbox"?event.target.checked:event.target.value);
          
        }
        else if (attr == "driver"){
          data["sensors"][index][attr] = event.target.value;
          data["sensors"][index]["config"] = configname["drivers"].filter(obj => {
            return obj.name === event.target.value
          })[0]["conf"];
        }
        else {
          data["sensors"][index][attr][configname] = event.target.value;

        }
        this.setState({ data });
      };
      // handleSensorConfigChange = (index,attr) => (event, newValue) => {
      //   console.log("handleSensorConfigChange: ",event.target,newValue);
      //   const { data } = this.state;
      //   data["sensors"][index]["config"][attr] = event.target.value;
      //   this.setState({ data });
      // };

      handleCloudChange = (attr) => (event, newValue) => {
        console.log("handleSensorChange: ", event.target, newValue);
        const { data } = this.state;
        data["cloudService"][attr] = event.target.value;
        this.setState({ data });
      };

      handleNewSensor = (event) => {
        console.log("New sensor: ");
        const { data } = this.state;
        if (data.sensors == null)
          data["sensors"] = [];
        data["sensors"].push({
          "name": "New sensor",
          "driver": "Random",
          "config": { "min": 0, "max": 30 },
          "interval": ""
        });
        this.setState({ data });
      };

      handleRemoveSensor = (index) => {
        console.log("remove sensor: ", index);
        const { data } = this.state;
        data["sensors"].splice(index, 1);
        this.setState({ data });
      };

      handleSliderChange = name => (event, newValue) => {
        const { data } = this.state;
        data[name] = newValue;
        this.setState({ data });
      };

      handleCheckboxChange = name => event => {
        const { data } = this.state;
        data[name] = event.target.checked;
        this.setState({ data });
      }

      render() {
        return <FormComponent
          handleValueChange={this.handleValueChange}
          handleCheckboxChange={this.handleCheckboxChange}
          handleSliderChange={this.handleSliderChange}
          handleSensorChange={this.handleSensorChange}
          handleCloudChange={this.handleCloudChange}
          handleNewSensor={this.handleNewSensor}
          handleRemoveSensor={this.handleRemoveSensor}
          setData={this.setData}
          saveData={this.saveData}
          loadData={this.loadData}
          {...this.state}
          {...this.props}
        />;
      }

    }
  );
}