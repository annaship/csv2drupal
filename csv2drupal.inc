<?php
// $Id: csv2drupal.inc, v 0.2 2011/02/18 ashipunova Exp ashipunova $

require_once 'includes/PHPExcel/IOFactory.php';
require_once 'csv2drupal_field_name_arrays.inc';
//module_load_include('module', 'geo', 'geo');

function get_data_from_excel_file($file) {
  $sheet_names = sheet_names();

  try {
    $obj_reader = PHPExcel_IOFactory::createReaderForFile($file);
  } catch (Exception $e) {
    $emes = $e->getMessage();
    dpr('Caught exception: ' .  $emes . "\n");
    dpm('Caught exception: ' .  $emes . "\n");
  }

  $obj_reader->setLoadSheetsOnly(0);
  $obj_reader->setReadDataOnly(true);
  $obj_php_excel = PHPExcel_IOFactory::load($file);

  $loaded_sheet_names = $obj_php_excel->getSheetNames();
  foreach ($loaded_sheet_names as $sheet_index => $loaded_sheet_name) {
    $lowloaded_sheet_name = strtolower($loaded_sheet_name);

    if (in_array($sheet_names[$lowloaded_sheet_name], $sheet_names)) {
      $obj_worksheet = $obj_php_excel->setActiveSheetIndex($sheet_index);
      $dim = $obj_worksheet->calculateWorksheetDimension();
      $highest_row = $obj_worksheet->getHighestRow();
      $highest_column = $obj_worksheet->getHighestColumn();
      $highest_column_index = PHPExcel_Cell::columnIndexFromString($highest_column);

      //get titles
      $titles = array();
      for ($col = 0; $col <= $highest_column_index; ++$col) {
        $titles[] = trim($obj_worksheet->getCellByColumnAndRow($col, "1")->getValue());
      }

      for ($row = 2; $row <= $highest_row; ++$row) {
        for ($col = 0; $col <= $highest_column_index; ++$col) {

          $res = trim($obj_worksheet->getCellByColumnAndRow($col, $row)->getValue());
            if (!empty($titles[$col])) {
              ${$sheet_names[$lowloaded_sheet_name]}[$titles[$col]][] = $res;
          }
        }
      }      
    }
    $content[$sheet_names[$lowloaded_sheet_name]] = ${$sheet_names[$lowloaded_sheet_name]};
  }
  return $content;
}

function create_nodes($content, $file_object) {
  $ref_nids = create_person_node($content['person'], &$ref_nids);
  $ref_nids = create_variable_node($content['variable'], &$ref_nids);
  $ref_nids = create_site_node($content['site'], &$ref_nids);
  $ref_nids = create_datafile_node($content['datafile'], &$ref_nids, $file_object);
  $ref_nids = create_dataset_node($content['dataset'], &$ref_nids);
  $keywords_taxonomy = create_keywords_node($content['keywords'], &$ref_nids);
  add_referrers($ref_nids);
  variable_set('ref_nids', $ref_nids);
  return $ref_nids;
}

function get_uid() {
  global $user;
  return $user->uid;
}

/*
 
 *
 */

//restructures arrays from Excel
function create_values($content) {
  $values = array();
  foreach ($content as $key => $value) {
    foreach ($value as $number => $data) {
      $clean_data = htmlspecialchars($data);
      $values[$number][$key] = $clean_data;
    }
  }
  return $values;
}

function add_node_value(&$node, $value, $fields_names) {
  foreach ($fields_names as $node_field_name => $csv_title) {
    $node->$node_field_name = array(array('value' => $value[$csv_title]));
  }
  return $node;
}

function validate_uniqness($value, $fields_names, $content_type) {
  $sql = $query = $nid = "";
  if (!empty ($value)) {
    $sql   = create_sql($value, $fields_names, $content_type);
    $query = db_query($sql);
    $nid   = db_result($query);
  }
  return $nid;
}

function create_sql($value, $fields_names, $content_type) {
  $full_field_name = "";
  $i = 0;
  $tags_to_title = tags_to_title();
  $content_type_titles_only = array(
    'data_file',
    'research_site',
    'data_set'
  );

  //    TODO: use field_person_email_email
  $forbiddenf_fields = forbiddenf_fields();
  $array_length  = count($fields_names) - count(array_intersect_key($fields_names, array_flip($forbiddenf_fields)));
  foreach ($fields_names as $field_name => $tag_name) {
    if (!in_array($field_name, $forbiddenf_fields)) {
      $i++;
      $full_field_name = $field_name . '_value';
      $condition .= "(" . $full_field_name . " = \"" . $value[$tag_name] . "\" OR " . $full_field_name . " is NULL)";
      if ($i < $array_length) {
        $condition .= " AND ";
      }
    }
  }
  $title_value = $value[$tags_to_title[$content_type]];
  if (in_array($content_type, $content_type_titles_only)) {
    $sql = "SELECT {node}.nid FROM {content_type_" . $content_type . "} as content_type
        JOIN {node} on content_type.nid = {node}.nid
        WHERE {node}.title = \"" . $title_value . "\";";
  }
  else {
    $sql   = "SELECT {node}.nid FROM {content_type_" . $content_type . "} as content_type
          JOIN {node} on content_type.nid = {node}.nid
          WHERE " . $condition . ";";
  }
  return $sql;
}

//TODO: add check for single vs. range
function convert_date_range($date_range) {
   if (preg_match_all("/(\w{1,3}).(\d{1,2}).(\d{2,4})/", $date_range, $matches, PREG_SET_ORDER)) {
    foreach ($matches as $match) {
      $date = str_replace('-', ' ', $match[0]);
      $dates[] = strftime("%G-%m-%d 00:00:00", strtotime($date));
    }
  }
  elseif (preg_match_all("/(\d{1,2}).(\d{1,2}).(\d{2,4})/", $date_range, $matches, PREG_SET_ORDER)) {
    foreach ($matches as $match) {
      $date = str_replace('-', '/', $match[0]);
      $dates[] = strftime("%G-%m-%d 00:00:00", strtotime($date));
    }
  }
  elseif (preg_match_all("/^(\d{4})$/", $date_range, $matches, PREG_SET_ORDER)) {
    foreach ($matches as $match) {
      $date = preg_replace("/^(\d{4})$/", "12/12/$1", $match[0]);
      $dates[] = strftime("%G-%m-%d 00:00:00", strtotime($date));
    }
  }
  return $dates;
}

function create_beg_end_date_arr($date_range_values) {
  $beg_end_date = array();
  $beg_end_date['value']  = $date_range_values[0];
  $beg_end_date['value2'] = $date_range_values[1];
  $beg_end_date += add_timezone_to_date_arr($beg_end_date);
  return $beg_end_date;
}

function create_single_date_arr($date_value) {
  $date_arr = array();
  $converted_value = convert_date_range($date_value);
  $date_arr['value'] = $converted_value[0];
  $date_arr += add_timezone_to_date_arr($date_arr);
  return $date_arr;
}

function add_timezone_to_date_arr(&$date_arr) {
  $date_arr['timezone']     = 'America/New_York';
  $date_arr['timezone_db']  = 'America/New_York';
  $date_arr['date_type']    = 'datetime';
  return $date_arr;
}

function create_person_node($content, &$ref_nids) {
  $content_type  = 'person';
  $persons_arr   = array();
  $person_values = create_values($content);
  $fields_names  = person_fields();
  foreach ($person_values as $value) {
    $value['Full Name'] = $value['First Name'] . " " . $value['Last Name'];
    $nid = validate_uniqness($value, $fields_names, $content_type);
    if (!empty($nid)) {
      $ref_nids[$content_type][] = $nid;
    }
    elseif (!empty($value['Last Name'])) {
      $node = new stdClass();
      $node->type  = $content_type;
      $node->uid   = get_uid();
      $full_name   = $value['Full Name'];
      $node->title = $full_name;
      $node = add_node_value($node, $value, $fields_names);
      $node->field_person_email[0]['email'] = $value['Email'];
      $node->field_person_uid[0]['uid'] = $value['person_uid'];
      $node->field_person_pubs[0]['nid'] = $value['Publications'];
      node_save(&$node);
      $person_nid = $node->nid;
      $ref_nids[$content_type][] = $person_nid;
    }
  }
  return $ref_nids;
}

function create_variable_node($content, &$ref_nids) {
  $content_type = 'variable';
  $variable_values = create_values($content);
  $tags_to_title   = tags_to_title();
  $title_label = $tags_to_title['variable'];
  $fields_names    = variable_fields();
  foreach ($variable_values as $value) {
    $nid = validate_uniqness($value, $fields_names, $content_type);
    if (!empty($nid)) {
      $ref_nids[$content_type][] = $nid;
    }    
    elseif (!empty($value[$title_label])) {
      $node        = new stdClass();
      $node->type  = $content_type;
      $node->uid   =  get_uid();
      $node->title = $value[$title_label];
      $node = add_node_value($node, $value, $fields_names);
      node_save(&$node);
      $ref_nids[$content_type][] = $node->nid;
    }
  }
  return $ref_nids;
}

function create_site_node($content, &$ref_nids) {
//  TODO: 2000 has "location" and "coordinates". Include!
  $content_type = 'research_site';
  $research_site_values = create_values($content);
  $fields_names = research_site_fields();
  foreach ($research_site_values as $value) {
    $nid = validate_uniqness($value, $fields_names, $content_type);
    if (!empty($nid)) {
      $ref_nids[$content_type][] = $nid;
    }
//    'Location' is used as a 'Title' if latter is absent
    elseif (!empty($value['Title']) or !empty($value['Location'])) {
      $node = new stdClass();
      $node->type  = $content_type;
      $node->uid   =  get_uid();
      !empty($value['Title']) ? $node->title = $value['Title'] : $node->title = $value['Location'];
      $node        = add_node_value($node, $value, $fields_names);
      
      node_save(&$node);
      $ref_nids[$content_type][] = $nid;


      if (preg_match("/(.+),\s*(.+)/", $value['Coordinates'], $matches)) {
        add_geo_to_node($node, $matches);
      }
    }
  }
  return $ref_nids;
}

function add_geo_to_node($node, $matches){
    $nid    = $node->nid;
    $coord1 = $matches[1];
    $coord2 = $matches[2];
    $coordinates = "POINT(" . $coord1 . " " . $coord2 . ")";
    $table_name = '';
    global $db_prefix;

    $table_name_query = "
      SELECT TABLE_NAME
      FROM INFORMATION_SCHEMA.COLUMNS
      WHERE column_name LIKE '%coords_geo%'
    ";
    $result = db_query(db_rewrite_sql($table_name_query));
    while ($data = db_result($result)) {
      if (preg_match("/^($db_prefix)(.+)$/", $data, $matches)) {
          $table_name = $matches[2];
      }
      else {
        $table_name = $data;
      }
    }
    $update_geo_query = "UPDATE {%s}
      SET field_research_site_pt_coords_geo=(SELECT GeomFromText('%s'))
      WHERE nid=%d";
    $db_result = db_query($update_geo_query, $table_name, $coordinates, $nid);
    
}


function create_file_object($file) {
  $details = stat($file);
  $filesize = $details['size'];

  // Get the path to your Drupal site's files directory
  $dest = file_directory_path();

  // Copy the file to the Drupal files directory
  if (!file_copy($file, $dest)) {
      echo "Failed to move file: $file.\n";
      return;
  }
  else {
      // file_move might change the name of the file
      $name = basename($file);
  }

  // Build the file object
  $file_obj = new stdClass();
  $file_obj->filename   = $name;
  $file_obj->filepath   = $file;
  $file_obj->filemime   = file_get_mimetype($name);
  $file_obj->filesize   = $filesize;
  $file_obj->filesource = $name;
  $file_obj->uid        = get_uid();
  $file_obj->status     = FILE_STATUS_TEMPORARY;
  $file_obj->timestamp  = time();
  $file_obj->list       = 1;
  $file_obj->new        = TRUE;

  // Save file to files table
  drupal_write_record('files', $file_obj);
  // change file status to permanent
  file_set_status($file_obj, 1);

  return $file_obj;
}

function create_datafile_node($content, &$ref_nids, $file_object) {
  $content_type     = 'data_file';
  $data_file_values = create_values($content);
  $fields_names     = datafile_fields();
  foreach ($data_file_values as $value) {
    $nid = validate_uniqness($value, $fields_names, $content_type);
    if (!empty($nid)) {
      $ref_nids[$content_type][] = $nid;
    }
    elseif (!empty($value['Data File Structure Name'])) {
      $node = new stdClass();
      $node->type   = $content_type;
      $node->uid    =  get_uid();
      $node->title  = $value['Data File Structure Name'];
      $node = add_node_value($node, $value, $fields_names);
      $node->field_beg_end_date[0] = create_beg_end_date_arr(convert_date_range($value['Date Range']));
      $node->field_beg_end_date[0]['date_type'] = 'datetime';
      if (!empty($ref_nids['variable'])) {
        foreach ($ref_nids['variable'] as $variable) {
          $node->field_datafile_variable_ref[]['nid'] = $variable;
        }
      }
      if (!empty($value['Sites']) && $ref_nids['research_site']) {
        foreach ($ref_nids['research_site'] as $dataset_site) {
          $node->field_datafile_site_ref[]['nid']     = $dataset_site;
        }
      }

      $node->field_data_file[0] = (array) $file_object;
      node_save(&$node);
      $ref_nids[$content_type][] = $node->nid;
    }
  }
  return $ref_nids;
}

function create_keywords_node($content, &$ref_nids) {
//    dpr("create_keywords_node");
    return $ref_nids;
}

function create_dataset_node($content, &$ref_nids) {
  $content_type   = 'data_set';
  $dataset_values = create_values($content);
  $fields_names   = dataset_fields();
  foreach ($dataset_values as $value) {
    $nid = validate_uniqness($value, $fields_names, $content_type);
    if (!empty($nid)) {
      $ref_nids[$content_type][] = $nid;
    }
    elseif (!empty($value['Title'])) {
      $node = new stdClass();
      $node->type                                       = $content_type;
      $node->uid                                        =  get_uid();
      $node->title                                      = $value['Title'];
      $node = add_node_value($node, $value, $fields_names);
      $node->field_beg_end_date[0]                      = create_beg_end_date_arr(convert_date_range($value['Date Range']));
      $node->field_beg_end_date[0]['date_type']         = 'datetime';
      $node->field_dataset_publication_date[0]          = create_single_date_arr($value['Publication Date']);
      $node->field_dataset_publication_date[0]['date_type'] = 'date';
//TODO: take ref for biblio
//    $node->field_dataset_biblio_ref[0]['value']       = $related_bibliography;
      foreach ($ref_nids['data_file'] as $data_file_nid) {
        $node->field_dataset_datafile_ref[]['nid']      = $data_file_nid;
      }
      $dataset_person_refs = get_person_ref($value, $ref_nids['person']);
      $add_person_refs     = add_person_refs(&$node, $dataset_person_refs);
      if ($ref_nids['research_site']) {
        foreach ($ref_nids['research_site'] as $dataset_site) {
          $node->field_dataset_site_ref[]['nid']        = $dataset_site;
        }
      }
      node_save(&$node);
      $ref_nids[$content_type][] = $node->nid;
    }
  }
  return $ref_nids;
}

function add_referrers($ref_nids) {
  $data_file_nids = $ref_nids['data_file'];

  save_ref_node($ref_nids['variable'], create_referrer($data_file_nids), 'field_datafile_variable_ref');
  save_ref_node($data_file_nids, create_referrer($ref_nids['data_set']), 'field_dataset_referrer');
}

function save_ref_node($nids, $referrers, $field_name) {
  if (!empty ($nids) AND !empty ($refferers)) {
    foreach ($nids as $nid) {
      foreach ($refferers as $refferer) {
        $r_node = node_load($nid);
        $r_node->$field_name = $refferer;
      }
      node_save(&$r_node);
    }
  }
}

function create_referrer($nids) {
  $reffers = array();
  if ($nids) {
    foreach ($nids as $nid) {
      $node = node_load($nid);
      $reffers[] = array(array(
        'items' => array(array(
        'nid'   => $node->nid,
        'vid'   => $node->vid,
        'title' => $node->title,
        ))
      ));
    }
  }
  return $reffers;
}

function display_report_page() {
  $csv2drupal_submit_time = variable_get(csv2drupal_submit_time, time());

  $sql = "SELECT nid FROM {node} WHERE created >= %d ORDER BY nid DESC";
  $db_result = db_query($sql, $csv2drupal_submit_time);

  $header       = array('Title', 'Nid');
  $rows['data'] = array();
  while ($res   = db_fetch_object($db_result)) {
    $nid        = $res->nid;
    $node       = node_load($nid);
    $node_nid   = $node->nid;
    $node_title = $node->title;
    $node_type  = $node->type;
    $node_link  = l($nid, 'node/' . $nid);
    if ($node_type != $old_node_type) {
      $rows[] = long_cell('Type = ' . $node_type);
      $old_node_type = $node_type;
    }
    $rows[] = array('Title = ' . $node_title, $node_link);
  }
  count($rows) == 1 ? $rows[] = long_cell('No new nodes was saved') : drupal_set_message('Followed nodes were just created:');
  variable_del(csv2drupal_submit_time);
  return theme_table($header, $rows);
}

function long_cell($long_cell_value) {
  $long_cell = array(array('data' => $long_cell_value, 'colspan' => 2,
          'font size' => 4, 'align' => 'center'));
  return $long_cell;
}

function _trim_text($str) {
    $str = trim($str);
    $str = preg_replace('/ +/', ' ', $str);
    return $str;
}

function get_person_array($data) {
  $person_names = array();
  if (preg_match_all("/([^,]+)/", $data, $matches, PREG_SET_ORDER)) {
    foreach ($matches as $match) {
      $person_names[] = _trim_text($match[1]);
    }
  }
  return $person_names;
}

function get_person_ref($dataset_value, $person_ref_nids) {
  $dataset_person_refs = array();
  $dataset_person_ref_fields = dataset_person_ref_fields();
  
  foreach ($dataset_person_ref_fields as $dataset_field_name => $dataset_role) {
    $name = get_person_array($dataset_value[$dataset_role]);
    is_array($name) ? $names = $name : $names = (array) $name;
    if (!empty ($person_ref_nids)) {
      foreach ($person_ref_nids as $person_ref_nid) {
        $node = node_load($person_ref_nid);

        $title = $node->title;
        if (in_array(trim($title), $names)) {
          $dataset_person_refs[$dataset_field_name][] = $person_ref_nid;
        }
      }
    }
  }
  return $dataset_person_refs;
}

function add_person_refs(&$node, $dataset_person_refs) {
  foreach ($dataset_person_refs as $field_name => $value1) {
    $refs = array();
    foreach ($value1 as $ref) {
      $refs[]['nid'] = $ref;
    }
    $node->$field_name = $refs;
  }
  return $node;
}
