var schema = {
  "ROOT": {
    "VAR": {
      "type": "object",
      "name": "ScriptVariable",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "value",
          "type": "Number",
          "required": false
        }
      ]
    },
    "CONSTVAR": {
      "type": "object",
      "name": "ScriptVariableReadOnly",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "value",
          "type": "Number",
          "required": false
        }
      ]
    },
    "WRITEVAR": {
      "type": "object",
      "name": "ScriptVariableWriteOnlyTest",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        }
      ]
    },
    "ARRAY": {
      "type": "object",
      "name": "ScriptArray",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "readonly",
          "type": "Bool",
          "required": false
        },
        {
          "name": "items",
          "type": "ArrayPrimitive",
          "required": true
        }
      ]
    },
    "DIN": {
      "type": "object",
      "name": "DigitalInput",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        }
      ]
    },
    "DOUT": {
      "type": "object",
      "name": "DigitalOutput",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        }
      ]
    },
    "DPOUT": {
      "type": "object",
      "name": "SinglePulseOutput",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "plength",
          "type": "UInt",
          "required": false,
          "min": 1,
          "max": 0,
          "default": 500
        },
        {
          "name": "activeLevel",
          "type": "String",
          "required": false,
          "allowedValues": [
            "low",
            "high"
          ],
          "default": "high"
        }
      ]
    },
    "ADC": {
      "type": "object",
      "name": "AnalogInput",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        }
      ]
    },
    "1WTG": {
      "type": "object",
      "name": "OneWireTempGroupAtRoot",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "refreshtime",
          "type": "OneOfGroup",
          "required": true,
          "fields": [
            {
              "name": "refreshtimems",
              "type": "Float",
              "required": false,
              "min": 0,
              "max": 0,
              "default": 0
            },
            {
              "name": "refreshtimesec",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            },
            {
              "name": "refreshtimemin",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            }
          ]
        },
        {
          "name": "items",
          "type": "Array",
          "required": true,
          "emptyPolicy": "Error",
          "subtype": {
            "type": "object",
            "name": "OneWireTempBus",
            "unknownPolicy": "Warn",
            "emptyPolicy": "Error",
            "fields": [
              {
                "name": "disabled",
                "type": "Bool",
                "required": false
              },
              {
                "name": "uid",
                "type": "UID",
                "required": true,
                "minLength": 1,
                "maxLength": 8,
                "default": ""
              },
              {
                "name": "note",
                "type": "String",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "pin",
                "type": "HardwarePin",
                "required": true
              },
              {
                "name": "items",
                "type": "Array",
                "required": true,
                "emptyPolicy": "Error",
                "subtype": {
                  "type": "object",
                  "name": "OneWireTempDevice",
                  "unknownPolicy": "Warn",
                  "emptyPolicy": "Error",
                  "fields": [
                    {
                      "name": "disabled",
                      "type": "Bool",
                      "required": false
                    },
                    {
                      "name": "uid",
                      "type": "UID",
                      "required": true,
                      "minLength": 1,
                      "maxLength": 8,
                      "default": ""
                    },
                    {
                      "name": "note",
                      "type": "String",
                      "required": false,
                      "minLength": 1,
                      "maxLength": 0,
                      "default": ""
                    },
                    {
                      "name": "romid",
                      "type": "HexBytes",
                      "required": true
                    }
                  ]
                }
              }
            ]
          }
        }
      ]
    },
    "1WTB": {
      "type": "object",
      "name": "OneWireTempBusAtRoot",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "refreshtime",
          "type": "OneOfGroup",
          "required": true,
          "fields": [
            {
              "name": "refreshtimems",
              "type": "Float",
              "required": false,
              "min": 0,
              "max": 0,
              "default": 0
            },
            {
              "name": "refreshtimesec",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            },
            {
              "name": "refreshtimemin",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            }
          ]
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "items",
          "type": "Array",
          "required": true,
          "emptyPolicy": "Error",
          "subtype": {
            "type": "object",
            "name": "OneWireTempDevice",
            "unknownPolicy": "Warn",
            "emptyPolicy": "Error",
            "fields": [
              {
                "name": "disabled",
                "type": "Bool",
                "required": false
              },
              {
                "name": "uid",
                "type": "UID",
                "required": true,
                "minLength": 1,
                "maxLength": 8,
                "default": ""
              },
              {
                "name": "note",
                "type": "String",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "romid",
                "type": "HexBytes",
                "required": true
              }
            ]
          }
        }
      ]
    },
    "1WTD": {
      "type": "object",
      "name": "OneWireTempDeviceAtRoot",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "refreshtime",
          "type": "OneOfGroup",
          "required": true,
          "fields": [
            {
              "name": "refreshtimems",
              "type": "Float",
              "required": false,
              "min": 0,
              "max": 0,
              "default": 0
            },
            {
              "name": "refreshtimesec",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            },
            {
              "name": "refreshtimemin",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            }
          ]
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "romid",
          "type": "HexBytes",
          "required": true
        }
      ]
    },
    "DHT": {
      "type": "object",
      "name": "DHT",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Error",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "model",
          "type": "String",
          "required": true,
          "allowedValues": [
            "DHT11",
            "DHT22",
            "AM2302",
            "RTH03"
          ],
          "default": "DHT11"
        },
        {
          "name": "refreshtime",
          "type": "OneOfGroup",
          "required": true,
          "fields": [
            {
              "name": "refreshtimems",
              "type": "Float",
              "required": false,
              "min": 0,
              "max": 0,
              "default": 0
            },
            {
              "name": "refreshtimesec",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            },
            {
              "name": "refreshtimemin",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            }
          ]
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        }
      ]
    },
    "TX433": {
      "type": "object",
      "name": "TX433",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "units",
          "type": "RegistryArray",
          "required": true,
          "regPath": "ROOT.TX433"
        }
      ]
    },
    "REGO600": {
      "type": "object",
      "name": "REGO600",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "rxpin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "txpin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "requestDelayMs",
          "type": "UInt",
          "required": false,
          "min": 0,
          "max": 0,
          "default": 10
        },
        {
          "name": "refreshtime",
          "type": "OneOfGroup",
          "required": true,
          "fields": [
            {
              "name": "refreshtimems",
              "type": "Float",
              "required": false,
              "min": 0,
              "max": 0,
              "default": 0
            },
            {
              "name": "refreshtimesec",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            },
            {
              "name": "refreshtimemin",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            }
          ]
        },
        {
          "name": "items",
          "type": "Array",
          "required": true,
          "emptyPolicy": "Error",
          "subtype": {
            "type": "object",
            "name": "REGO600_Register",
            "unknownPolicy": "Warn",
            "emptyPolicy": "Warn",
            "fields": [
              {
                "name": "disabled",
                "type": "Bool",
                "required": false
              },
              {
                "name": "uid",
                "type": "UID",
                "required": true,
                "minLength": 1,
                "maxLength": 8,
                "default": ""
              },
              {
                "name": "note",
                "type": "String",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "regname",
                "type": "StringConstraint",
                "required": true,
                "allowedValues": [
                  "GT1",
                  "GT2",
                  "GT3",
                  "GT4",
                  "GT5",
                  "GT6",
                  "GT8",
                  "GT9",
                  "GT10",
                  "GT11",
                  "GT3x",
                  "P3",
                  "COMP",
                  "EL3",
                  "EL6",
                  "P1",
                  "P2",
                  "VXV",
                  "ALARM"
                ],
                "default": ""
              }
            ]
          }
        }
      ]
    },
    "I2C": {
      "type": "object",
      "name": "I2C_Master",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "sckpin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "sdapin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "items",
          "type": "RegistryArray",
          "required": true,
          "regPath": "ROOT.I2C_Master"
        }
      ]
    },
    "WS2812": {
      "type": "object",
      "name": "WS2812",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "ledcount",
          "type": "UInt",
          "required": true,
          "min": 1,
          "max": 0,
          "default": 1
        },
        {
          "name": "format",
          "type": "String",
          "required": true,
          "allowedValues": [
            "RGB",
            "RBG",
            "GRB",
            "GBR",
            "BRG",
            "BGR",
            "WRGB",
            "WRBG",
            "WGRB",
            "WGBR",
            "WBRG",
            "WBGR",
            "RWGB",
            "RWBG",
            "GWRB",
            "GWBR",
            "BWRG",
            "BWGR",
            "RGWB",
            "RBWG",
            "GRWB",
            "GBWR",
            "BRWG",
            "BGWR",
            "RGBW",
            "RBGW",
            "GRBW",
            "GBRW",
            "BRGW",
            "BGRW"
          ],
          "default": "RGB"
        },
        {
          "name": "ifspeed",
          "type": "String",
          "required": false,
          "allowedValues": [
            "KHZ800",
            "KHZ400"
          ],
          "default": "KHZ800"
        },
        {
          "name": "brightness",
          "type": "UInt",
          "required": false,
          "min": 1,
          "max": 127,
          "default": 127
        },
        {
          "name": "mode",
          "type": "UInt",
          "required": false,
          "min": 0,
          "max": 80,
          "default": 0
        },
        {
          "name": "fxspeed",
          "type": "UInt",
          "required": false,
          "min": 0,
          "max": 65535,
          "default": 3000
        }
      ]
    },
    "CONTAINER": {
      "type": "object",
      "name": "DeviceContainer",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "items",
          "type": "RegistryArray",
          "required": true,
          "regPath": "ROOT"
        }
      ]
    },
    "THINGSPEAK": {
      "type": "object",
      "name": "ThingSpeak",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "refreshtime",
          "type": "OneOfGroup",
          "required": false,
          "fields": [
            {
              "name": "refreshtimems",
              "type": "Float",
              "required": false,
              "min": 0,
              "max": 0,
              "default": 0
            },
            {
              "name": "refreshtimesec",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            },
            {
              "name": "refreshtimemin",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            }
          ]
        },
        {
          "name": "items",
          "type": "Object",
          "required": true,
          "object": true,
          "subtype": {
            "type": "object",
            "name": "items",
            "unknownPolicy": "Error",
            "emptyPolicy": "Error",
            "fields": [
              {
                "name": "1",
                "type": "UID_Path",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "2",
                "type": "UID_Path",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "3",
                "type": "UID_Path",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "4",
                "type": "UID_Path",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "5",
                "type": "UID_Path",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "6",
                "type": "UID_Path",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "7",
                "type": "UID_Path",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "8",
                "type": "UID_Path",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              }
            ]
          }
        }
      ]
    },
    "HOMEASSISTANT": {
      "type": "object",
      "name": "HomeAssistant",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "modes": [
        {
          "name": "global group mode",
          "conjunctions": [
            {
              "name": "group",
              "required": true
            },
            {
              "name": "items",
              "required": true
            },
            {
              "name": "groups",
              "required": false
            }
          ]
        },
        {
          "name": "individual groups mode",
          "conjunctions": [
            {
              "name": "group",
              "required": false
            },
            {
              "name": "items",
              "required": false
            },
            {
              "name": "groups",
              "required": true
            }
          ]
        }
      ],
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "deviceId",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "host",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "port",
          "type": "UInt",
          "required": true,
          "min": 1,
          "max": 65535,
          "default": 1883
        },
        {
          "name": "credentials",
          "type": "AllOfGroup",
          "required": false,
          "fields": [
            {
              "name": "user",
              "type": "String",
              "required": false,
              "minLength": 1,
              "maxLength": 0,
              "default": ""
            },
            {
              "name": "pass",
              "type": "String",
              "required": false,
              "minLength": 1,
              "maxLength": 0,
              "default": ""
            }
          ]
        },
        {
          "name": "group",
          "type": "Object",
          "required": false,
          "object": true,
          "subtype": {
            "type": "object",
            "name": "GlobalGroup",
            "unknownPolicy": "Warn",
            "emptyPolicy": "Warn",
            "fields": [
              {
                "name": "uid",
                "type": "UID",
                "required": true,
                "minLength": 1,
                "maxLength": 8,
                "default": ""
              },
              {
                "name": "name",
                "type": "String",
                "required": true,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              }
            ]
          }
        },
        {
          "name": "items",
          "type": "RegistryArray",
          "required": false,
          "regPath": "ROOT.HOMEASSISTANT"
        },
        {
          "name": "groups",
          "type": "Array",
          "required": false,
          "emptyPolicy": "Warn",
          "subtype": {
            "type": "object",
            "name": "IndividualGroup",
            "unknownPolicy": "Warn",
            "emptyPolicy": "Warn",
            "fields": [
              {
                "name": "uid",
                "type": "UID",
                "required": true,
                "minLength": 1,
                "maxLength": 8,
                "default": ""
              },
              {
                "name": "name",
                "type": "String",
                "required": true,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "items",
                "type": "RegistryArray",
                "required": false,
                "regPath": "ROOT.HOMEASSISTANT"
              }
            ]
          }
        }
      ]
    },
    "ACTUATOR": {
      "type": "object",
      "name": "Actuator",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "modes": [
        {
          "name": "h-bridge ab mode",
          "conjunctions": [
            {
              "name": "hbridgeModeAB",
              "required": true
            },
            {
              "name": "hbridgeModeOC",
              "required": false
            },
            {
              "name": "dir_enableMode",
              "required": false
            },
            {
              "name": "pinBreak",
              "required": false
            }
          ]
        },
        {
          "name": "h-bridge open close mode",
          "conjunctions": [
            {
              "name": "hbridgeModeAB",
              "required": false
            },
            {
              "name": "hbridgeModeOC",
              "required": true
            },
            {
              "name": "dir_enableMode",
              "required": false
            },
            {
              "name": "pinBreak",
              "required": false
            }
          ]
        },
        {
          "name": "dir/enable mode",
          "conjunctions": [
            {
              "name": "hbridgeModeAB",
              "required": false
            },
            {
              "name": "hbridgeModeOC",
              "required": false
            },
            {
              "name": "dir_enableMode",
              "required": true
            },
            {
              "name": "pinBreak",
              "required": false
            }
          ]
        },
        {
          "name": "dir/enable/break mode",
          "conjunctions": [
            {
              "name": "hbridgeModeAB",
              "required": false
            },
            {
              "name": "hbridgeModeOC",
              "required": false
            },
            {
              "name": "dir_enableMode",
              "required": true
            },
            {
              "name": "pinBreak",
              "required": true
            }
          ]
        }
      ],
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "hbridgeModeAB",
          "type": "AllOfGroup",
          "required": false,
          "fields": [
            {
              "name": "pinA",
              "type": "HardwarePin",
              "required": false
            },
            {
              "name": "pinB",
              "type": "HardwarePin",
              "required": false
            }
          ]
        },
        {
          "name": "hbridgeModeOC",
          "type": "AllOfGroup",
          "required": false,
          "fields": [
            {
              "name": "pinOpen",
              "type": "HardwarePin",
              "required": false
            },
            {
              "name": "pinClose",
              "type": "HardwarePin",
              "required": false
            }
          ]
        },
        {
          "name": "dir_enableMode",
          "type": "AllOfGroup",
          "required": false,
          "fields": [
            {
              "name": "pinDir",
              "type": "HardwarePin",
              "required": false
            },
            {
              "name": "pinEnable",
              "type": "HardwarePin",
              "required": false
            }
          ]
        },
        {
          "name": "pinBreak",
          "type": "HardwarePin",
          "required": false
        },
        {
          "name": "MinEndStop",
          "type": "Object",
          "required": false,
          "object": true,
          "subtype": {
            "type": "object",
            "name": "InputPinScheme",
            "unknownPolicy": "Warn",
            "emptyPolicy": "Warn",
            "fields": [
              {
                "name": "pin",
                "type": "HardwarePin",
                "required": true
              },
              {
                "name": "ActiveHigh",
                "type": "Bool",
                "required": false
              }
            ]
          }
        },
        {
          "name": "MaxEndStop",
          "type": "Object",
          "required": false,
          "object": true,
          "subtype": {
            "type": "object",
            "name": "InputPinScheme",
            "unknownPolicy": "Warn",
            "emptyPolicy": "Warn",
            "fields": [
              {
                "name": "pin",
                "type": "HardwarePin",
                "required": true
              },
              {
                "name": "ActiveHigh",
                "type": "Bool",
                "required": false
              }
            ]
          }
        },
        {
          "name": "timeoutMs",
          "type": "UInt",
          "required": false,
          "min": 1,
          "max": 0,
          "default": 10000
        }
      ]
    },
    "RELAY_LATCHING": {
      "type": "object",
      "name": "LatchingRelay",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "modes": [
        {
          "name": "direct ab mode",
          "conjunctions": [
            {
              "name": "directModeAB",
              "required": true
            },
            {
              "name": "directModeOC",
              "required": false
            },
            {
              "name": "dir_enableMode",
              "required": false
            }
          ]
        },
        {
          "name": "direct set reset mode",
          "conjunctions": [
            {
              "name": "directModeAB",
              "required": false
            },
            {
              "name": "directModeOC",
              "required": true
            },
            {
              "name": "dir_enableMode",
              "required": false
            }
          ]
        },
        {
          "name": "dir/enable mode",
          "conjunctions": [
            {
              "name": "directModeAB",
              "required": false
            },
            {
              "name": "directModeOC",
              "required": false
            },
            {
              "name": "dir_enableMode",
              "required": true
            }
          ]
        }
      ],
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "directModeAB",
          "type": "AllOfGroup",
          "required": false,
          "fields": [
            {
              "name": "pinA",
              "type": "HardwarePin",
              "required": false
            },
            {
              "name": "pinB",
              "type": "HardwarePin",
              "required": false
            }
          ]
        },
        {
          "name": "directModeOC",
          "type": "AllOfGroup",
          "required": false,
          "fields": [
            {
              "name": "pinSet",
              "type": "HardwarePin",
              "required": false
            },
            {
              "name": "pinRst",
              "type": "HardwarePin",
              "required": false
            }
          ]
        },
        {
          "name": "dir_enableMode",
          "type": "AllOfGroup",
          "required": false,
          "fields": [
            {
              "name": "pinDir",
              "type": "HardwarePin",
              "required": false
            },
            {
              "name": "pinEnable",
              "type": "HardwarePin",
              "required": false
            }
          ]
        },
        {
          "name": "ResetState",
          "type": "Object",
          "required": false,
          "object": true,
          "subtype": {
            "type": "object",
            "name": "InputPinScheme",
            "unknownPolicy": "Warn",
            "emptyPolicy": "Warn",
            "fields": [
              {
                "name": "pin",
                "type": "HardwarePin",
                "required": true
              },
              {
                "name": "ActiveHigh",
                "type": "Bool",
                "required": false
              }
            ]
          }
        },
        {
          "name": "SetState",
          "type": "Object",
          "required": false,
          "object": true,
          "subtype": {
            "type": "object",
            "name": "InputPinScheme",
            "unknownPolicy": "Warn",
            "emptyPolicy": "Warn",
            "fields": [
              {
                "name": "pin",
                "type": "HardwarePin",
                "required": true
              },
              {
                "name": "ActiveHigh",
                "type": "Bool",
                "required": false
              }
            ]
          }
        },
        {
          "name": "timeoutMs",
          "type": "UInt",
          "required": false,
          "min": 1,
          "max": 0,
          "default": 500
        }
      ]
    },
    "PWM_SERVO": {
      "type": "object",
      "name": "PWM_Servo",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "constraints": [
        {
          "fieldA": "minVal",
          "type": "<",
          "fieldB": "maxVal"
        },
        {
          "fieldA": "minPulseLength",
          "type": "<",
          "fieldB": "maxPulseLength"
        },
        {
          "fieldA": "minPulseLength",
          "type": "<=",
          "fieldB": "startPulseLength"
        },
        {
          "fieldA": "startPulseLength",
          "type": "<=",
          "fieldB": "maxPulseLength"
        }
      ],
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "ch",
          "type": "UInt",
          "required": true,
          "min": 0,
          "max": 7,
          "default": 0
        },
        {
          "name": "minPulseLength",
          "type": "UInt",
          "required": false,
          "min": 100,
          "max": 20000,
          "default": 1000
        },
        {
          "name": "maxPulseLength",
          "type": "UInt",
          "required": false,
          "min": 100,
          "max": 20000,
          "default": 2000
        },
        {
          "name": "startPulseLength",
          "type": "UInt",
          "required": false,
          "min": 100,
          "max": 20000,
          "default": 1500
        },
        {
          "name": "autoOffAfterMs",
          "type": "UInt",
          "required": false,
          "min": 0,
          "max": 0,
          "default": 0
        },
        {
          "name": "pulseLengthOffset",
          "type": "UInt",
          "required": false,
          "min": 0,
          "max": 0,
          "default": 0
        },
        {
          "name": "minVal",
          "type": "Float",
          "required": false,
          "min": null,
          "max": null,
          "default": 0
        },
        {
          "name": "maxVal",
          "type": "Float",
          "required": false,
          "min": null,
          "max": null,
          "default": 100
        }
      ]
    },
    "BUTTON": {
      "type": "object",
      "name": "ButtonInput",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        },
        {
          "name": "debounceMs",
          "type": "UInt",
          "required": false,
          "min": 1,
          "max": 0,
          "default": 30
        },
        {
          "name": "activeLevel",
          "type": "String",
          "required": false,
          "allowedValues": [
            "low",
            "high"
          ],
          "default": "high"
        },
        {
          "name": "on_press",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        }
      ]
    },
    "TEMPLATE": {
      "type": "object",
      "name": "_Template_",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "pin",
          "type": "HardwarePin",
          "required": true
        }
      ]
    }
  },
  "ROOT.TX433": {
    "lc": {
      "type": "object",
      "name": "TX433_Unit_TypeLC",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "modes": [
        {
          "name": "AlphaNumeric ID mode",
          "conjunctions": [
            {
              "name": "anid",
              "required": true
            },
            {
              "name": "hexid",
              "required": false
            }
          ]
        },
        {
          "name": "hex ID mode",
          "conjunctions": [
            {
              "name": "anid",
              "required": false
            },
            {
              "name": "hexid",
              "required": true
            }
          ]
        }
      ],
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "anid",
          "type": "String",
          "required": false,
          "minLength": 4,
          "maxLength": 4,
          "default": "Id01"
        },
        {
          "name": "hexid",
          "type": "String",
          "required": false,
          "minLength": 6,
          "maxLength": 6,
          "default": "090A0B"
        },
        {
          "name": "grp_btn",
          "type": "UInt",
          "required": false,
          "min": 0,
          "max": 3,
          "default": 0
        },
        {
          "name": "btn",
          "type": "UInt",
          "required": false,
          "min": 0,
          "max": 3,
          "default": 0
        },
        {
          "name": "state",
          "type": "UInt",
          "required": false,
          "min": 0,
          "max": 1,
          "default": 0
        }
      ]
    },
    "sfc": {
      "type": "object",
      "name": "TX433_Unit_TypeSFC",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "ch",
          "type": "UInt",
          "required": false,
          "min": 1,
          "max": 4,
          "default": 0
        },
        {
          "name": "btn",
          "type": "UInt",
          "required": false,
          "min": 1,
          "max": 4,
          "default": 0
        },
        {
          "name": "state",
          "type": "UInt",
          "required": false,
          "min": 0,
          "max": 1,
          "default": 0
        }
      ]
    },
    "afc": {
      "type": "object",
      "name": "TX433_Unit_TypeAFC",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "ch",
          "type": "String",
          "required": false,
          "allowedValues": [
            "0",
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "A",
            "B",
            "C",
            "D",
            "E",
            "F"
          ],
          "default": "0"
        },
        {
          "name": "btn",
          "type": "String",
          "required": false,
          "allowedValues": [
            "0",
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "A",
            "B",
            "C",
            "D",
            "E",
            "F"
          ],
          "default": "0"
        },
        {
          "name": "state",
          "type": "UInt",
          "required": false,
          "min": 0,
          "max": 1,
          "default": 0
        }
      ]
    }
  },
  "ROOT.I2C_Master": {
    "SSD1306": {
      "type": "object",
      "name": "Display_SSD1306",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "width",
          "type": "UInt",
          "required": true,
          "min": 8,
          "max": 128,
          "default": 128
        },
        {
          "name": "height",
          "type": "UInt",
          "required": true,
          "min": 8,
          "max": 64,
          "default": 64
        },
        {
          "name": "addr",
          "type": "HexBytes",
          "required": true
        },
        {
          "name": "items",
          "type": "Array",
          "required": true,
          "emptyPolicy": "Warn",
          "subtype": {
            "type": "object",
            "name": "SSD1306 element",
            "unknownPolicy": "Warn",
            "emptyPolicy": "Warn",
            "fields": [
              {
                "name": "disabled",
                "type": "Bool",
                "required": false
              },
              {
                "name": "uid",
                "type": "UID",
                "required": true,
                "minLength": 1,
                "maxLength": 8,
                "default": ""
              },
              {
                "name": "x",
                "type": "UInt",
                "required": true,
                "min": 0,
                "max": 128,
                "default": 0
              },
              {
                "name": "y",
                "type": "UInt",
                "required": true,
                "min": 0,
                "max": 64,
                "default": 0
              },
              {
                "name": "label",
                "type": "String",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              },
              {
                "name": "source",
                "type": "UID_Path",
                "required": false,
                "minLength": 1,
                "maxLength": 0,
                "default": ""
              }
            ]
          }
        }
      ]
    },
    "PCF8574x": {
      "type": "object",
      "name": "PCF8574x",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "addr",
          "type": "HexBytes",
          "required": true
        }
      ]
    }
  },
  "ROOT.HOMEASSISTANT": {
    "sensor": {
      "type": "object",
      "name": "HA_Sensor",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "modes": [
        {
          "name": "refresh mode",
          "conjunctions": [
            {
              "name": "refreshtime",
              "required": true
            },
            {
              "name": "source",
              "required": true
            },
            {
              "name": "event_source",
              "required": false
            }
          ]
        },
        {
          "name": "event mode",
          "conjunctions": [
            {
              "name": "refreshtime",
              "required": false
            },
            {
              "name": "source",
              "required": true
            },
            {
              "name": "event_source",
              "required": true
            }
          ]
        },
        {
          "name": "script mode",
          "conjunctions": [
            {
              "name": "refreshtime",
              "required": false
            },
            {
              "name": "source",
              "required": false
            },
            {
              "name": "event_source",
              "required": false
            }
          ]
        }
      ],
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "refreshtime",
          "type": "OneOfGroup",
          "required": false,
          "fields": [
            {
              "name": "refreshtimems",
              "type": "Float",
              "required": false,
              "min": 0,
              "max": 0,
              "default": 0
            },
            {
              "name": "refreshtimesec",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            },
            {
              "name": "refreshtimemin",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            }
          ]
        },
        {
          "name": "name",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "discovery",
          "type": "Object",
          "required": false,
          "object": true
        }
      ]
    },
    "binary_sensor": {
      "type": "object",
      "name": "HA_BinarySensor",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "modes": [
        {
          "name": "refresh mode",
          "conjunctions": [
            {
              "name": "refreshtime",
              "required": true
            },
            {
              "name": "source",
              "required": true
            },
            {
              "name": "event_source",
              "required": false
            }
          ]
        },
        {
          "name": "event mode",
          "conjunctions": [
            {
              "name": "refreshtime",
              "required": false
            },
            {
              "name": "source",
              "required": true
            },
            {
              "name": "event_source",
              "required": true
            }
          ]
        },
        {
          "name": "script mode",
          "conjunctions": [
            {
              "name": "refreshtime",
              "required": false
            },
            {
              "name": "source",
              "required": false
            },
            {
              "name": "event_source",
              "required": false
            }
          ]
        }
      ],
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "refreshtime",
          "type": "OneOfGroup",
          "required": false,
          "fields": [
            {
              "name": "refreshtimems",
              "type": "Float",
              "required": false,
              "min": 0,
              "max": 0,
              "default": 0
            },
            {
              "name": "refreshtimesec",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            },
            {
              "name": "refreshtimemin",
              "type": "Float",
              "required": false,
              "min": null,
              "max": null,
              "default": 1
            }
          ]
        },
        {
          "name": "name",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "discovery",
          "type": "Object",
          "required": false,
          "object": true
        }
      ]
    },
    "switch": {
      "type": "object",
      "name": "HA_Switch",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "target",
          "type": "UID_Path",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "name",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "discovery",
          "type": "Object",
          "required": false,
          "object": true
        }
      ]
    },
    "button": {
      "type": "object",
      "name": "HA_Button",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "target",
          "type": "UID_Path",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "name",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "discovery",
          "type": "Object",
          "required": false,
          "object": true
        }
      ]
    },
    "number": {
      "type": "object",
      "name": "HA_Number",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "target",
          "type": "UID_Path",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "name",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "discovery",
          "type": "Object",
          "required": false,
          "object": true
        }
      ]
    },
    "CONTAINER": {
      "type": "object",
      "name": "HA_DeviceContainer",
      "unknownPolicy": "Warn",
      "emptyPolicy": "Warn",
      "fields": [
        {
          "name": "disabled",
          "type": "Bool",
          "required": false
        },
        {
          "name": "uid",
          "type": "UID",
          "required": true,
          "minLength": 1,
          "maxLength": 8,
          "default": ""
        },
        {
          "name": "note",
          "type": "String",
          "required": false,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "type",
          "type": "String",
          "required": true,
          "minLength": 1,
          "maxLength": 0,
          "default": ""
        },
        {
          "name": "items",
          "type": "RegistryArray",
          "required": true,
          "regPath": "ROOT.HOMEASSISTANT"
        }
      ]
    }
  }
};