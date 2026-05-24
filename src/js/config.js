// Clay configuration for PebbleConduct
// This defines the settings UI

module.exports = [
  {
    "type": "heading",
    "defaultValue": "PebbleConduct Settings"
  },
  {
    "type": "text",
    "defaultValue": "Configure your train information data source."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "select",
        "messageKey": "dataSource",
        "label": "Data Source",
        "defaultValue": "tcs",
        "options": [
          { "label": "TCS RailDesk", "value": "tcs" },
          { "label": "whereisthe.es", "value": "whereisthees" },
          { "label": "DB RIS", "value": "db_ris" },
          { "label": "NS (Nederlandse Spoorwegen)", "value": "ns" }
        ]
      },
      {
        "type": "input",
        "messageKey": "trainNumber",
        "label": "Train Number (Path UUID for TCS)",
        "defaultValue": ""
      },
      {
        "type": "input",
        "messageKey": "commercialTrainNumber",
        "label": "Commercial Train Number to display",
        "defaultValue": ""
      },
      {
        "type": "input",
        "messageKey": "date",
        "label": "Date (YYYY-MM-DD)",
        "defaultValue": ""
      },
      {
        "type": "select",
        "messageKey": "updateInterval",
        "label": "Update Interval",
        "defaultValue": "60",
        "options": [
          { "label": "15 seconds", "value": "15" },
          { "label": "30 seconds", "value": "30" },
          { "label": "45 seconds", "value": "45" },
          { "label": "60 seconds", "value": "60" },
          { "label": "120 seconds", "value": "120" }
        ]
      }
    ]
  },
  {
    "type": "section",
    "label": "TCS RailDesk Settings",
    "items": [
      {
        "type": "text",
        "defaultValue": "TCS RailDesk Settings"
      },
      {
        "type": "input",
        "messageKey": "tcsUrl",
        "label": "TCS RailDesk URL",
        "defaultValue": ""
      },
      {
        "type": "input",
        "messageKey": "tcsUsername",
        "label": "Username",
        "defaultValue": ""
      },
      {
        "type": "input",
        "messageKey": "tcsPassword",
        "label": "Password",
        "defaultValue": "",
        "attributes": {
          "type": "password"
        }
      }
    ]
  },
  {
    "type": "section",
    "label": "DB RIS Settings",
    "items": [
      {
        "type": "text",
        "defaultValue": "DB RIS Settings"
      },
      {
        "type": "input",
        "messageKey": "dbClientId",
        "label": "DB Client ID",
        "defaultValue": ""
      },
      {
        "type": "input",
        "messageKey": "dbApiKey",
        "label": "DB API Key",
        "defaultValue": "",
        "attributes": {
          "type": "password"
        }
      }
    ]
  },
  {
    "type": "section",
    "label": "NS (Nederlandse Spoorwegen) Settings",
    "items": [
      {
        "type": "text",
        "defaultValue": "NS (Nederlandse Spoorwegen) Settings"
      },
      {
        "type": "input",
        "messageKey": "nsSubscriptionKey",
        "label": "NS Subscription Key (Get your API key from https://apiportal.ns.nl/)",
        "defaultValue": "",
        "attributes": {
          "type": "password"
        }
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
