R"(
<!DOCTYPE html>
<html>

<head>
  <meta charset="utf-8">
  <title>SliderGUI</title>
  <meta name="viewport" content="width=device-width">
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css" integrity="sha384-1BmE4kWBq78iYhFldvKuhfTAU6auU8tT94WrHftjDbrCEXSU1oBoqyl2QvZ6jIW3" crossorigin="anonymous">
  <style type="text/css" id="gwd-text-style">
    p {
      margin: 0px;
    }
    h1 {
      margin: 0px;
    }
    h2 {
      margin: 0px;
    }
    h3 {
      margin: 0px;
    }
  </style>
  <style type="text/css">
    html, body {
      width: 100%;
      height: 100%;
      margin: 0px;
    }
    body {
      background-color: transparent;
    }
    .gwd-div-juoq {
      left: 0px;
      top: 0px;
    }
    .gwd-jog-button {
      color: rgb(255, 255, 255);
      font-size: 50px;
    }
    .bottom-labels {}
    .top-label {
      font-size: 1em;
    }
    .preventTapToZoom {
    touch-action: manipulation;
    -webkit-user-select: none;
    user-select: none;
    }
  </style>
</head>

<body class="htmlNoPages">
  <div class="container-fluid d-flex flex-column col-12 gwd-div-juoq" id="MainDiv">
    <div class="row g-3 h-40" style="padding-top: 1em; padding-bottom: 1em;" id="TopSection">
      <div class="d-grid col-8 g-1" id="TopSettings">
        <p class="display-6" style="font-size: 1.3em;">Current position:</p>
        <h1 class="display-1" id="currentPosLabel">0.000 mm</h1>
        <p class="display-6 top-label">Jog increment:</p>
        <div class="btn-group" role="group" aria-label="Basic radio toggle button group" >
          <input type="radio" class="btn-check preventTapToZoom" name="btnradio" id="buttonJogIncrFast" autocomplete="off" onchange='simpleAjaxCall("jogIncrFast")'>
          <label class="btn btn-outline-secondary" for="buttonJogIncrFast">1.0 mm</label>

          <input type="radio" class="btn-check preventTapToZoom" name="btnradio" id="buttonJogIncrMedium" autocomplete="off" onchange='simpleAjaxCall("jogIncrMedium")'>
          <label class="btn btn-outline-secondary" for="buttonJogIncrMedium">0.5 mm</label>

          <input type="radio" class="btn-check preventTapToZoom" name="btnradio" id="buttonJogIncrSlow" autocomplete="off" onchange='simpleAjaxCall("jogIncrSlow")' checked>
          <label class="btn btn-outline-secondary" for="buttonJogIncrSlow">0.1 mm</label>
        </div>
        <p class="display-6 top-label">Number of images:</p>
        <input class="form-control bottom-labels" type="number" placeholder="eg. 200" value=200 id="numImagesForm" onchange='formContentChanged(numImagesForm)'>
      </div>
      <div class="d-grid gap-2 col-4 mx-auto" id="TopButtons">
        <button type="button" class="btn btn-primary btn-floating btn-lg gwd-jog-button preventTapToZoom" data-mdb-ripple-color="#ffffff" id="buttonJogFwd" onpointerup='simpleAjaxCall("jogFwd")'>↑</button>
        <button type="button" class="btn btn-primary btn-floating btn-lg gwd-jog-button preventTapToZoom" data-mdb-ripple-color="#ffffff" id="buttonJogBwd" onpointerup='simpleAjaxCall("jogBwd")'>↓</button>
      </div>
    </div>
    <div style="padding-right: 0.7em;/*! padding-top: 1em; */ /*! padding-bottom: 1em; */" id="MidSection" class="row g-4 h-10">
      <div class="d-grid gap-2 col-5" id="MidLeft">
        <button type="button" class="btn btn-lg btn-primary bottom-labels preventTapToZoom" style="font-size: 1rem;" onpointerup='simpleAjaxCall("setStartPoint")'>Set startpoint</button>
        <button type="button" class="btn btn-lg btn-primary bottom-labels preventTapToZoom" style="font-size: 1rem;" onpointerup='simpleAjaxCall("setEndPoint")'>Set endpoint</button>
        <button type="button" class="btn btn-lg btn-secondary bottom-labels preventTapToZoom" style="font-size: 1rem;" onpointerup='simpleAjaxCall("testImage")'>Test image</button>
      </div>
      <div class="container d-flex flex-column col-7 gap-2" id="MidRight">
        <div class="row flex-grow-1">
          <button type="button" class="btn btn-lg btn-success bottom-labels preventTapToZoom" onpointerup='simpleAjaxCall("start")'>Start shooting</button>
        </div>
        <div class="row h-33">
          <button type="button" class="btn btn-lg btn-danger bottom-labels preventTapToZoom" style="font-size: 1rem;" onpointerup='simpleAjaxCall("abort")'>Abort shooting</button>
        </div>
      </div>
    </div>
    <div class="row g-1 flex-grow-1" style="padding-top: 1em; padding-bottom: 1em;" id="BottomSection">
      <h6 class="h6 bottom-labels" id="Statslabel">Stats:</h6>
      <div class="col-4 bottom-labels">Total movement distance</div>
      <div class="col-4 bottom-labels" id="distanceStatLabel">0.0 mm</div>
      <div class="col-4 bottom-labels"></div>
      <div class="col-4 bottom-labels">Increment</div>
      <div class="col-4 bottom-labels" id="incrementStatLabel">0.0 mm</div>
      <div class="col-4 bottom-labels"></div>
      <div class="col-4 bottom-labels">Total shooting time</div>
      <div class="col-4 bottom-labels" id="shootTimeStatLabel">0.0 min</div>
      <div class="col-4 bottom-labels"></div>
      <h6 class="h6 bottom-labels" id="settingsLabel">Settings:</h6>
      <div class="col-4 bottom-labels">Shooting speed</div>
      <div class="col-3 bottom-labels">
        <input class="form-control bottom-labels" type="number" placeholder="eg. 0.1" value=0.1 id="shootingSpeedForm" onchange='formContentChanged(shootingSpeedForm)'>
      </div>
      <div class="col-3 bottom-labels"> mm/s</div>
      <div class="col-2 bottom-labels"></div>
      <div class="col-4 bottom-labels">Jog speed</div>
      <div class="col-3 bottom-labels">
        <input class="form-control  bottom-labels" type="number" placeholder="eg. 1" value=1 id="jogSpeedForm" onchange='formContentChanged(jogSpeedForm)'>
      </div>
      <div class="col-3 bottom-labels"> mm/s</div>
      <div class="col-2 bottom-labels"></div>
      <div class="col-4 bottom-labels">Deshake delay</div>
      <div class="col-3 bottom-labels">
        <input class="form-control bottom-labels" type="number" placeholder="eg. 4" value=4 id="deshakeDelayForm" onchange='formContentChanged(deshakeDelayForm)'>
      </div>
      <div class="col-3 bottom-labels"> s</div>
      <div class="col-2 bottom-labels"></div>
      <div class="col-4 bottom-labels">Shoot delay</div>
      <div class="col-3 bottom-labels">
        <input class="form-control bottom-labels" type="number" placeholder="eg. 0.5" value=0.5 id="shootDelayForm" onchange='formContentChanged(shootDelayForm)'>
      </div>
      <div class="col-3 bottom-labels"> s</div>
      <div class="col-2 bottom-labels"></div>
    </div>
  </div>
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.min.js" integrity="sha384-QJHtvGhmr9XOIpI6YVutG+2QOK9T+ZnN4kzFN1RtK3zEFEIsxhlmWl5/YESvpZ13" crossorigin="anonymous"></script>
  <script> 
  window.onload = refreshLabels
  window.setInterval(refreshLabels, 5000)

  function refreshLabels() {
    $.ajax({
    "url": "refreshLabels",
    "success": function( result ) {
      console.log("Ajax result: "+result);
      var arr = result.split(":");
      var key = arr[0]
      var value = -1;
      for (let i = 1; i < arr.length; i++) { 
        let arr2 = arr[i].split(",")
        value = arr2[0];
        switch (key) {
          case "currentPos":
            currentPosLabel.textContent = value; 
            break;
          case "distance":
            distanceStatLabel.textContent = value;
            break;
          case "increment":
            incrementStatLabel.textContent = value;
            break;
          case "shootingTime":
            shootTimeStatLabel.textContent = value;
            break;
          default:
            break;
        }

        key = arr2[1];
      }
    } 
  });
  }

  function formContentChanged(formID){
    if(formID.validity.valid === true){
      makeAjaxCall(formID.id,
      {
        "num": formID.value
      });
      console.log(formID.id+" changed");
      refreshLabels();
    }else{
      console.log("Oopsie."+formID.id+" contains invalid data");
    }
  }

  function makeAjaxCall(url,data){
    $.ajax({
    "url": url,
    "data": data,
    "success": function( result ) {
      console.log("Ajax result: "+result);
    } 
  });
  }
  function simpleAjaxCall(url){
    $.ajax({"url": url});
    refreshLabels();
  }
</script>
</body>

</html>
)"