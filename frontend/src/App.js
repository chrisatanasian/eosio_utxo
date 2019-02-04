import React, { Component } from 'react';
import './App.css';

class App extends Component {
  constructor(props) {
    super(props);
    this.state = {
      from: '',
      to: '',
      amount: 0,
      fee: 1,
      nonce: 1,
      memo: '',
    };
  }

  targetValue = (target) => {
    switch(target.type) {
      case "number":
        return target.valueAsNumber;
      case "text":
        return target.value;
      default:
        throw new Error("Unexpected target type");
    }
  }

  handleChange = (event) => {
    this.setState({ [event.target.name]: this.targetValue(event.target) });
  }

  handleSubmit = (event) => {
    console.log(this.state);
    event.preventDefault();
  }

  isSubmitDisabled() {
    return this.state.from === '' ||
           this.state.to === '' ||
           isNaN(this.state.amount) ||
           isNaN(this.state.fee) ||
           isNaN(this.state.nonce);
  }

  render() {
    return (
      <div className="App">
        <form onSubmit={this.handleSubmit}>
          <label>
            From Public Key:
            <input type="text"
                   value={this.state.from}
                   name="from"
                   placeholder="Enter from public key"
                   onChange={this.handleChange} />
          </label>
          <label>
            To Public Key:
            <input type="text"
                   value={this.state.to}
                   name="to"
                   placeholder="Enter to public key"
                   onChange={this.handleChange} />
          </label>
          <label>
            Amount:
            <input type="number"
                   value={this.state.amount}
                   name="amount"
                   onChange={this.handleChange} />
          </label>
          <label>
            Fee:
            <input type="number"
                   value={this.state.fee}
                   name="fee"
                   onChange={this.handleChange} />
          </label>
          <label>
            Nonce:
            <input type="number"
                   value={this.state.nonce}
                   name="nonce"
                   onChange={this.handleChange} />
          </label>
          <label>
            Memo:
            <input type="text"
                   value={this.state.memo}
                   name="memo"
                   onChange={this.handleChange} />
          </label>
          <input type="submit" value="Submit" disabled={this.isSubmitDisabled()} />
        </form>
      </div>
    );
  }
}

export default App;
