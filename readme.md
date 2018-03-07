# description
A demo of Nama ([a SDK about face](http://www.faceunity.com)).

# functions
- multiple input sources: camera, image, video etc.
- input source's aspect: vertical vs. horizontal
- adding controls according to a json file (resources/ctrl_config.json). The controls is used to set parameters of Nama's props
- no limits to props used
- auto load avaliable props
- saving results

# screenshots
![screenshot 0](screenshots/screenshot_0.png)
![screenshot 1](screenshots/screenshot_1.png)
![screenshot 2](screenshots/screenshot_2.png)


# requirement
- VS2015
- Qt 5.*
- OpenCV
- [Nama SDK](https://github.com/Faceunity)
- some of my code: [YXL](https://github.com/cx2200252/YXL_code/tree/master/YXL)  

# ctrl_config.json

All controls' setting are in node "params". Each sub-node within "params" is a page in the applications.
```C
{
  "params":{
    "page0":[
      {
        //1th control        
      },
      {
        //2th control
      }
    ],
    "page1":[
      {
        //3th control
      }
    ]
    ...
  }
}
```
## control configuration
- checkbox
```C
{
    "type": "checkbox",
    "show_name": "is_beauty_on",
    "tooltip": "is_beauty_on",
    "param": "is_beauty_on",
    "prop_idx": 0,
    "val": false
}
```
- slider
```C
{
    "type": "slider",
    "show_name": "美白",
    "tooltip": "",
    "param": "color_level",
    "prop_idx": 0,
    "val": 0,
    "scale": 0.01,
    "range": [0, 100]
}
```
- combobox
```C
{
    "type": "combobox",
    "show_name": "filter",
    "tooltip": "",
    "param": "filter_name",
    "prop_idx": 0,
    "val": "nature",
    "combo_texts": ["nature", "delta", "electric", "slowlived"]
}
```
