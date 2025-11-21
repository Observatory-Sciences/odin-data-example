
odin_data = {
  api_version: '0.1',
  ctrl_name: 'detector',
  current_page: '.home-view',
  adapter_list: [],
  adapter_objects: {},
  ctrl_connected: false,
  fp_connected: [false,false,false,false],
  acq_id: '',
  daq: null
  };


String.prototype.replaceAll = function(search, replacement) {
    var target = this;
    return target.replace(new RegExp(search, 'g'), replacement);
};

$.put = function(url, data, callback, type)
{
  if ( $.isFunction(data) ){
    type = type || callback,
    callback = data,
    data = {}
  }

  return $.ajax({
    url: url,
    type: 'PUT',
    success: callback,
    data: data,
    contentType: type
  });
}

function render(url)
{
  // This function decides what type of page to show
  // depending on the current url hash value.
  // Get the keyword from the url.
  var temp = "." + url.split('/')[1];
  // Hide whatever page is currently shown.
  $(".page").hide();

  // Show the new page
  $(temp).show();
  odin_data.current_page = temp;

}


$( document ).ready(function() 
{
  $("#fp-tabs").tabs();
  update_api_version();
  update_api_adapters();
  render(decodeURI(window.location.hash));

  setInterval(update_api_version, 5000);
  setInterval(update_detector_status, 1000);
  setInterval(update_fp_status, 1000);

  $(document).on("keypress", "fp-set-path", function(e){
        if(e.which == 13){
            var inputVal = $(this).val();
            alert("You've entered: " + inputVal);
        }
    });

  $('#set-exposure').change(function(){
    update_exposure();
  });

  $('#set-frames').change(function(){
    update_frames();
  });

  $('#start-cmd').click(function(){
    ctrl_command('start');
  });

  $('#stop-cmd').click(function(){
    ctrl_command('stop');
  });

  $('#fp-set-frames').on('keypress',function(e) {
    if(e.which == 13) {
      send_fp_config({'frames': parseInt($('#fp-set-frames').val())});
    }
  });

  $('#fp-set-frames').change(function(){
    send_fp_config({'frames': parseInt($('#fp-set-frames').val())});
  });

  $('#fp-set-path').on('keypress',function(e) {
    if(e.which == 13) {
      send_fp_config({'file': {'path': $('#fp-set-path').val()}});
    }
  });

  $('#fp-set-path').change(function(){
    send_fp_config({'file': {'path': $('#fp-set-path').val()}});
  });

  $('#fp-set-prefix').on('keypress',function(e) {
    if(e.which == 13) {
      send_fp_config({'file': {'prefix': $('#fp-set-prefix').val()}});
    }
  });

  $('#fp-set-prefix').change(function(){
    send_fp_config({'file': {'prefix': $('#fp-set-prefix').val()}});
  });

  $('#fp-set-ext').on('keypress',function(e) {
    if(e.which == 13) {
      send_fp_config({'file': {'extension': $('#fp-set-ext').val()}});
    }
  });

  $('#fp-set-ext').change(function(){
    send_fp_config({'file': {'extension': $('#fp-set-ext').val()}});
  });

  $('#fp-start-cmd').click(function(){
    fp_command('start_writing');
  });

  $('#fp-stop-cmd').click(function(){
    fp_command('stop_writing');
  });

  $(window).on('hashchange', function(){
    // On every hash change the render function is called with the new hash.
	// This is how the navigation of the app is executed.
	render(decodeURI(window.location.hash));
  });
});

function process_cmd_response(response)
{
}

function update_exposure() {
    $.ajax({
        type: 'PUT',
        url: '/api/' + odin_data.api_version + '/' + odin_data.ctrl_name + '/config',
        contentType: 'application/json',
        data: JSON.stringify({'exposure_time': parseFloat($('#set-exposure').val())}),
    });
}

function update_frames() {
    $.ajax({
        type: 'PUT',
        url: '/api/' + odin_data.api_version + '/' + odin_data.ctrl_name + '/config',
        contentType: 'application/json',
        data: JSON.stringify({'frames': parseInt($('#set-frames').val())}),
    });
}

function ctrl_command(command) {
    $.ajax({
        type: 'PUT',
        url: '/api/' + odin_data.api_version + '/' + odin_data.ctrl_name + '/command',
        contentType: 'application/json',
        data: JSON.stringify({'execute': command}),
    });
}

function send_fp_config(config)
{
    $.ajax({
        type: 'PUT',
        url: '/api/' + odin_data.api_version + '/fp/0/config/hdf',
        contentType: 'application/json',
        data: JSON.stringify(config),
    });
}

function fp_command(command) {
    $.ajax({
        type: 'PUT',
        url: '/api/' + odin_data.api_version + '/fp/0/command/hdf',
        contentType: 'application/json',
        data: JSON.stringify({'execute': command}),
    });
}


function send_fr_command(command, params)
{
    $.ajax({
        url: '/api/' + odin_data.api_version + '/fr/config/' + command,
        type: 'PUT',
        dataType: 'json',
        data: params,
        headers: {'Content-Type': 'application/json',
            'Accept': 'application/json'},
        success: process_cmd_response,
        async: false
    });
}

function fr_reset_statistics()
{
    $.ajax({
        url: '/api/' + odin_data.api_version + '/fr/command/reset_statistics',
        type: 'PUT',
        dataType: 'json',
        headers: {'Content-Type': 'application/json',
            'Accept': 'application/json'},
        success: process_cmd_response,
        async: false
    });
}

function fp_reset_statistics()
{
    $.ajax({
        url: '/api/' + odin_data.api_version + '/fp/command/reset_statistics',
        type: 'PUT',
        dataType: 'json',
        headers: {'Content-Type': 'application/json',
            'Accept': 'application/json'},
        success: process_cmd_response,
        async: false
    });
}

function fp_debug_command() {
    send_fp_command('debug_level', $('#fp-debug-level').val());
}

function fr_debug_command() {
    send_fr_command('debug_level', $('#fp-debug-level').val());
}

function fp_mode_command(mode) {
    send_fp_command('tristan', JSON.stringify({
        "mode": mode.toLowerCase()
    }));
}

function meta_start_command() {
    send_fp_command('tristan', JSON.stringify({
        "acq_id": $('#set-fp-filename').val()
    }));
    odin_data.acq_id = $('#set-fp-filename').val();
    send_meta_command("acquisition_id", $('#set-fp-filename').val());
    send_meta_command("directory", $('#set-fp-path').val());
    send_meta_command("file_prefix", $('#set-fp-filename').val());
}

function fp_start_command() {
    send_fp_command('hdf', JSON.stringify({
        "acquisition_id": $('#set-fp-filename').val(),
        "file": {
            "name": $('#set-fp-filename').val(),
            "path": $('#set-fp-path').val()
        }
    }));
    send_fp_command('hdf', JSON.stringify({
        "write": true
    }));
}

function fp_raw_mode_command() {
    send_fp_command('tristan', JSON.stringify({
        "raw_mode": 1
    }));
}

function fp_process_mode_command() {
    send_fp_command('tristan', JSON.stringify({
        "raw_mode": 0
    }));
}

function fp_stop_command() {
    send_fp_command('hdf', JSON.stringify({
        "write": false
    }));
}

function update_api_version() {

    $.getJSON('/api', function(response) {
        $('#api-version').html(response.api);
        odin_data.api_version = response.api;
    });
}

function update_api_adapters() {
    $.getJSON('/api/' + odin_data.api_version + '/adapters/', function(response) {
        odin_data.adapter_list = response.adapters;
        adapter_list_html = response.adapters.join(", ");
        $('#api-adapters').html(adapter_list_html);
        //update_adapter_objects();
    });
}

function update_detector_status() {
    $.getJSON('/api/' + odin_data.api_version + '/' + odin_data.ctrl_name, function(response) {
        $('#get-exposure').html(response.config.exposure_time);
        $('#get-frames').html(response.config.frames);
        $('#status-acquiring').html(led_html(''+response.status.acquiring, 'green', 26));
        $('#status-frames').html(response.status.frames);
    });
}

function update_fp_status() {
    $.getJSON('/api/' + odin_data.api_version + '/fp/0', function(response) {
        $('#fp-status-connected').html(led_html(''+response['0'].status.connected, 'green', 26));
        $('#fp-get-frames').html(response['0'].config.hdf.frames);
        $('#fp-get-path').html(response['0'].config.hdf.file.path);
        $('#fp-get-prefix').html(response['0'].config.hdf.file.prefix);
        $('#fp-get-ext').html(response['0'].config.hdf.file.extension);
        $('#fp-status-writing').html(led_html(''+response['0'].status.hdf.writing, 'green', 26));
        $('#fp-status-file').html(response['0'].status.hdf.file_path + '/' + response['0'].status.hdf.file_name);
        $('#fp-status-frames').html(response['0'].status.hdf.frames_written);
    });
}

function update_fr_status() {
    $.getJSON('/api/' + odin_data.api_version + '/fr/status/buffers/empty', function (response) {
        //alert(JSON.stringify(response));
        odin_data.daq.setFREmptyBuffers(response['value']);
    });

    $.getJSON('/api/' + odin_data.api_version + '/fr/status/decoder/packets', function (response) {
        odin_data.daq.setFRPackets(response['value']);
        total_pkts = 0;
        for (var index = 0; index < response['value'].length; index++){
            total_pkts += parseInt(response['value'][index]);
        }
        $('#fr-pkts-received').html(''+total_pkts);
    });
    $.getJSON('/api/' + odin_data.api_version + '/fr/status/connected', function(response) {
        odin_data.daq.setFRConnected(response['value']);
    });
}

function led_html(value, colour, width)
{
  var html_text = "<img width=" + width + "px src=img/";
  if (value == 'true' || value === true){
    html_text +=  colour + "-led-on";
  } else {
    html_text += "led-off";
  }
  html_text += ".png></img>";
  return html_text;
}
