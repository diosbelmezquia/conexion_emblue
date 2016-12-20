<?php

/**
 * @file
 * Funciones para gestionar datos de conexion con emBlue
 *
 */

/**
 * Funcion para Autenticarse y poder hacer uso de la API
 * Devuelve Token dinamico
 */
function alqueria_emblue_autenticar($correo_emblue, $pass_emblue, $token_emblue) {
  // Inicializar una sesion cURL
  $curl = curl_init('http://api.embluemail.com/Services/EmBlue3Service.svc/Json/Authenticate');
  $curl_post_data = array(
    'User' => $correo_emblue,
    'Pass' => $pass_emblue,
    'Token' => $token_emblue
  );
  $token_dinamico = alqueria_emblue_curl($curl, $curl_post_data);
  $obj = json_decode($token_dinamico);

  //Devolver token dinamico que permite llamar a las funciones
  return $obj->Token;
}

/**
 * Funcion para validar que el enlace establecido a emBlue continue correctamente abierto
 */
function alqueria_emblue_conexion($token_dinamico) {
  // Inicializar una sesion cURL
  $curl = curl_init('http://api.embluemail.com/Services/EmBlue3Service.svc/Json/CheckConnection');
  $curl_post_data = array(
    'Token' => $token_dinamico
  );
  $obj = json_decode(alqueria_emblue_curl($curl, $curl_post_data));
  return $obj->Result;
}

/**
 * Funcion para agregar a emBlue nuevo usuario registrado en Alqueria
 */
function alqueria_emblue_agregar_usuario($token_dinamico, $email, $id_grupo_emblue, $datos_usuario) {
  // Inicializar una sesion cURL
  $curl = curl_init('http://api.embluemail.com/Services/EmBlue3Service.svc/Json/NewContact');
  $curl_post_data = array(
    'Token' => $token_dinamico,
    'Email' => $email,
    'SelectGroups' => $id_grupo_emblue
  );
  $result = alqueria_emblue_curl($curl, $curl_post_data);
  $obj = json_decode($result);

  //Si el correo es nuevo agregar datos adicionales de usuario
  if($obj->Description == "nuevo") {
    //ID del correo del usuario para agregar campos
    $id_email = $obj->EmailId;

    //Agregar datos adicionales del usuario, nombre, apellido, genero...  
    alqueria_emblue_agregar_campos($id_email, $datos_usuario, $token_dinamico);
  }
}

/**
 * Funcion para agregar/editar campos adicionales del usuario a emBlue
 */
function alqueria_emblue_agregar_campos($id_email, $datos_usuario, $token_dinamico) {
  // Inicializar una sesion cURL
  $curl = curl_init('http://api.embluemail.com/Services/EmBlue3Service.svc/Json/EditCustomFieldsOneContact');
  $curl_post_data = array(
    'EmailId' => $id_email,
    'EditedFields' => "nombre:|:".$datos_usuario[0].":|:1|||apellido:|:".$datos_usuario[1].":|:1|||cumpleanios:|:".$datos_usuario[2].":|:1|||sexo:|:".$datos_usuario[3].":|:1|||ciudad:|:".$datos_usuario[4].":|:1",
    'Token' => $token_dinamico
  );
  $result = alqueria_emblue_curl($curl, $curl_post_data);
}

/**
 * Funcion para mostrar todos los grupos de emBlue
 */
function alqueria_emblue_listado_grupos($token_dinamico) {
  // Inicializar una sesion cURL
  $curl = curl_init('http://api.embluemail.com/Services/EmBlue3Service.svc/Json/SearchGroup');
  $curl_post_data = array(
    'Token' => $token_dinamico,
    'FirstResult' => 0,
    'Search' => '',
    'Order' => 'asc'
  );
  $result = alqueria_emblue_curl($curl, $curl_post_data);
  $obj = json_decode($result);
  $options = array();
    foreach ($obj as $grupo) {
      $options[$grupo->GroupId] = $grupo->Name;
    }
  return $options;
}


/**
 * Funcion para configurar y ejecutar las llamadas a las funciones
 */
function alqueria_emblue_curl($curl, $curl_post_data) {
  //Convertir de Array a string JSON
  $jsonArray = json_encode($curl_post_data);

  //Establecer todas las opciones para la transferencia de datos
  curl_setopt($curl, CURLOPT_HTTPHEADER, array('Content-Type: application/json'));
  curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
  curl_setopt($curl, CURLOPT_POST, true);
  curl_setopt($curl, CURLOPT_POSTFIELDS, $jsonArray);

  //Ejecutar la url y la funcion pasada por $curl
  return curl_exec($curl);
}

/**
 * Funcion para mostrar el formulario de autenticacion con emBlue
 */
function alqueria_emblue_admin($form, &$form_state) {
  //Si es la segunda pagina, mostrar select con listado de grupo
  //La autenticacion fue correcta en la primera pagina
  if (!empty($form_state['page_num']) && $form_state['page_num'] == 2) {
    return alqueria_emblue_admin_2($form, $form_state);
  }
  $form_state['page_num'] = 1;

  $form['config_conexion'] = array(
    '#type' => 'fieldset',
    '#title' => 'Autenticarse para usar los servicios de emBlue',
  );
  $form['config_conexion']['correo_emblue'] = array(
    '#type' => 'textfield',
    '#title' => 'Correo',
    '#description' => 'Direccion de correo electronico emBlue.',
  );
  $form['config_conexion']['pass_emblue'] = array(
    '#type' => 'password',
    '#title' => 'Password',
    '#description' => 'Si ya ha introducido su contraseña antes, puede dejar este campo en blanco, a menos que desee cambiar la contraseña almacenada.',
  );
  $form['config_conexion']['token_emblue'] = array(
    '#type' => 'textfield',
    '#title' => 'Token emBlue',
    '#description' => 'Llave de acceso que emBlue le otorga a un usuario / cliente para poder realizar las consultas a la aplicacion.',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Autenticar',
  );
  return $form;
}

/**
 * Funcion para validar los datos el formulario de autenticacion con emBlue
 */
/*
function alqueria_emblue_admin_validate($form, &$form_state) {
  if ($form_state['page_num'] == 1) {
    if (!valid_email_address($form_state['values']['correo_emblue'])) {
      form_set_error('correo_emblue', 'Debe especificar una direccion de correo electronico valida');
    }
    if (empty($form_state['values']['pass_emblue']) && empty(variable_get('pass_emblue'))) {
      form_set_error('pass_emblue', 'Debe especificar una contraseña valida');
    }
    if (empty($form_state['values']['token_emblue'])) {
      form_set_error('token_emblue', 'El campo Token emBlue no puede estar vacio');
    }
  }
} 
*/

/**
 * Funcion para validar los datos el formulario de autenticacion con emBlue
 */
function alqueria_emblue_admin_validate($form, &$form_state) {
  if ($form_state['page_num'] == 1) {
    if (!valid_email_address($form_state['values']['correo_emblue'])) {
      form_set_error('correo_emblue', 'Debe especificar una direccion de correo electronico valida');
    }
    if (($form_state['values']['pass_emblue'] == "") && (variable_get('pass_emblue') == "")) {
      form_set_error('pass_emblue', 'Debe especificar una contraseña valida');
    }
    if ($form_state['values']['token_emblue'] == "") {
      form_set_error('token_emblue', 'El campo Token emBlue no puede estar vacio');
    }
  }
}

/**
 * Funcion para guardar datos de autenticacion con emBlue
 */
function alqueria_emblue_admin_submit($form, &$form_state) {
  $correo_emblue = $form_state['values']['correo_emblue'];
  $pass_emblue = $form_state['values']['pass_emblue'];
  $token_emblue = $form_state['values']['token_emblue'];
  $token_dinamico = '';

  empty($pass_emblue) ? $pass_emblue = variable_get('pass_emblue') : null;
  $token_dinamico = alqueria_emblue_autenticar($correo_emblue, $pass_emblue, $token_emblue);

  if (!empty($token_dinamico) && alqueria_emblue_conexion($token_dinamico)) {
    variable_set('correo_emblue', $correo_emblue);
    variable_set('pass_emblue', $pass_emblue);
    variable_set('token_emblue', $token_emblue);
    variable_set('token_dinamico', $token_dinamico);

    $form_state['page_values'][1] = $form_state['values'];
      if (!empty($form_state['page_values'][2])) {
        $form_state['values'] = $form_state['page_values'][2];
      }
    $form_state['page_num'] = 2;
    $form_state['rebuild'] = TRUE;
    drupal_set_message("Conexion a emBlue correcta");
  }
  else {
    drupal_set_message("No se pudo establecer la conexion con emBlue, verifique los datos de conexion",'error');
  }
}

/**
 * Funcion para mostrar el listado con todos los grupos de usuario en emBlue
 */
function alqueria_emblue_admin_2($form, &$form_state) {
  $form['config_grupo'] = array(
    '#type'  => 'fieldset',
    '#title' => t('Grupos de emBlue'),
  );
  $form['config_grupo']['grupos_emblue'] = array(
    '#type' => 'select',
    '#title' => 'Selecciona un grupo',
    '#default_value' => '65892',
    '#options' => alqueria_emblue_listado_grupos(variable_get('token_dinamico')),
    '#description' => 'Selecciona el grupo donde se guardaran los datos del usuario',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Guardar configuracion',
    '#submit' => array('alqueria_emblue_admin_2_submit'),
  );
  return $form;
}

/**
 * Funcion para guardar ID del grupo de usuario en emBlue
 */
function alqueria_emblue_admin_2_submit($form, &$form_state) {
  $id_grupo = $form_state['values']['grupos_emblue'];
  variable_set('id_grupo_emblue', $id_grupo);
  drupal_set_message("Grupo registrado");
}
