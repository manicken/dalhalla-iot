window.addEventListener('load', init);

function postData(url, data, onOK, onError)
{
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.onload = function() {
    if (xmlHttp.status != 200) { 
      if (onError != undefined && (typeof onError) == "function") onError();
      else console.log("post file fail:" + url);
    }
    else {
      if (onOK != undefined && (typeof onOK) == "function") onOK();
      else console.log("post file ok:" + url);
    }
  };
  var formData = new FormData();
  formData.append("data", new Blob([data], {type:undefined}));
  xmlHttp.open("POST", url);
  xmlHttp.send(formData);
}

function init()
{
  document.querySelectorAll('.sendBtn').forEach((button, index) => {
    button.addEventListener('click', () => {
      const input = document.querySelectorAll('.cmdInput')[index];
      const value = input.value;
      
      const select = document.querySelectorAll('.cmdTarget')[index];
      const target = select.value;
      //console.log(`Input value from row ${index + 1}:`, value);
      sendCmd(target, value);
    });
  });
}

function sendCmdOk()
{
  setState("cmd sent ok!");
}

function sendCmdFail()
{
  setState("cmd send fail");
}

function sendCmd(target, value)
{
  if (value.lenght !== 0)
    var json = `{"cmd":"${target}",${value}}`;
  else
    var json = `{"cmd":"${target}"}`;
  console.log(json);
  postData("../../json_cmd", json, sendCmdOk, sendCmdFail);
}


function setState(msg) {
    document.getElementById("info").innerHTML = msg;
}