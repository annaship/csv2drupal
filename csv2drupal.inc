<?php

require_once 'includes/PHPExcel/IOFactory.php';
require_once 'csv2drupal_field_name_arrays.inc';
//module_load_include('module', 'geo', 'geo');

function get_data_from_excel_file($file) {

  $sheet_names = sheet_names();
  
  $objReader = PHPExcel_IOFactory::createReaderForFile($file);
  $objReader->setLoadSheetsOnly(0);
  $objReader->setReadDataOnly(true);
  $objPHPExcel = PHPExcel_IOFactory::load($file);

  $loadedSheetNames = $objPHPExcel->getSheetNames();
  foreach ($loadedSheetNames as $sheetIndex => $loadedSheetName) {
    $objWorksheet = $objPHPExcel->setActiveSheetIndex($sheetIndex);
    $dim = $objWorksheet->calculateWorksheetDimension();
    $highestRow = $objWorksheet->getHighestRow();
    $highestColumn = $objWorksheet->getHighestColumn();
    $highestColumnIndex = PHPExcel_Cell::columnIndexFromString($highestColumn);

    //get titles
    $titles = array();
    for ($col = 0; $col <= $highestColumnIndex; ++$col) {
      $titles[] = trim($objWorksheet->getCellByColumnAndRow($col, "1")->getValue());
    }

    for ($row = 2; $row <= $highestRow; ++$row) {
      for ($col = 0; $col <= $highestColumnIndex; ++$col) {
        $res = trim($objWorksheet->getCellByColumnAndRow($col, $row)->getValue());
        if (!empty($loadedSheetName)) {
          $lowloadedSheetName = strtolower($loadedSheetName);
          if (!empty($titles[$col])) {
            ${$sheet_names[$lowloadedSheetName]}[$titles[$col]][] = $res;
          }
        }
      }
    }
    if (in_array($sheet_names[$lowloadedSheetName], $sheet_names)) {
      $content[$sheet_names[$lowloadedSheetName]] = ${$sheet_names[$lowloadedSheetName]};
    }
  }
//  dpr($content);
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
//  TODO link referrer
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
      $values[$number][$key] = $data;
    }
  }
  return $values;
}

function add_node_value(&$node, $value, $fields_names) {
//  dpm($value);
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


  //    TODO: check for other CCT
  //    TODO: use field_person_email_email
  $forbiddenf_fields = forbiddenf_fields();
  $array_length  = count($fields_names) - count(array_intersect_key($fields_names, array_flip($forbiddenf_fields)));
  foreach ($fields_names as $field_name => $tag_name) {
    if (!in_array($field_name, $forbiddenf_fields)) {
      $i++;
      $full_field_name = $field_name . '_value';
      $condition .= "(" . $full_field_name . " = '" . $value[$tag_name] . "' OR " . $full_field_name . " is NULL)";
      if ($i < $array_length) {
        $condition .= " AND ";
      }
    }
  }
  $title_value = $value[$tags_to_title[$content_type]];
  if (in_array($content_type, $content_type_titles_only)) {
    $sql = "SELECT node.nid FROM {content_type_" . $content_type . "} as content_type
        JOIN {node} on content_type.nid = node.nid
        WHERE node.title = '" . $title_value . "';";
  }
  else {
    $sql   = "SELECT node.nid FROM {content_type_" . $content_type . "} as content_type
          JOIN {node} on content_type.nid = node.nid
          WHERE " . $condition . ";";
  }
  return $sql;
}

/*
   foreach ($fields_names as $field_name => $tag_name) {
    if (!in_array($field_name, $forbiddenf_fields)) {
      $full_field_name = $field_name . '_value';
      $condition .= "(" . $full_field_name . " = '" . $value[$tag_name] . "' OR " . $full_field_name . " is NULL) ";
      $i++;
      if ($i < $array_length) {
        $condition .= " AND ";
      }
  }
  }
  $title_value = $value[$tags_to_title[$content_type]];
  $sql   = "SELECT node.nid FROM {content_type_" . $content_type . "} as content_type
          JOIN {node} on content_type.nid = node.nid
          WHERE node.title = '" . $title_value . "' OR " . $condition . ";";

 */

//TODO change format:
/*
 * Jul-23-2000 - Aug-01-2000
dd-mmm-yyyy format
 *  * Publication Date	field_dataset_publication_date	Datetime
 * Date Range	field_beg_end_date	Date
 *
 *  */

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
//  dpr($dates);
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
  $date_arr['date_type']    = 'datetime'; // TODO: or 'date', ask Inigo
  return $date_arr;
}

/*
 function _mymodule_somepage_callback($nid) {
  $query = db_query("SELECT * FROM {mytablename}");

  $data = array();

  $i = 0;
  while ($row = db_fetch_array($query)) {
    $data[$i] = $row;
    $i++;
  }

  $output = theme_table(array('ID', 'Look', 'Taste'), $data);
  return $output;
}

 */

function create_person_node($content, &$ref_nids) {
  $content_type  = 'person';
  $persons_arr   = array ();
  $person_values = create_values($content);
  $fields_names  = person_fields();
  foreach ($person_values as $value) {
    $value['Full Name'] = $value['First Name'] . " " . $value['Last Name'];
    $nid = validate_uniqness($value, $fields_names, $content_type);
    if (!empty($nid)) {
      $ref_nids[$content_type][] = $nid;
    }
    elseif (!empty($value['Last Name'])) {
      $node = new StdClass();
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
      $node        = new StdClass();
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
    elseif (!empty($value['Title'])) {
      $node = new StdClass();
      $node->type  = $content_type;
      $node->uid   =  get_uid();
      $node->title = $value['Title'];
      $node        = add_node_value($node, $value, $fields_names);
//      dpr($value['Coordinates']);
//      dpm($value['Coordinates']);
      if (preg_match("/(.+),\s(.+)/", $value['Coordinates'], $matches)) {
        $coord1 = $matches[1];
        $coord2 = $matches[2];
        $coordinates = "POINT(" . $coord1 . " " . $coord2 . ")";
        $q_geo       = db_result(db_query("SELECT GeomFromText('%s')", $coordinates));
//        $q_geo       = db_result(db_query("SELECT GeomFromText('POINT(68.6167 -149.31)')"));
//        $node->field_research_site_pt_coords[0]['geo'] = $coordinates;
//        $node->field_research_site_pt_coords[0]['geo'] = $q_geo;
      }

      /*
       * user warning: Incorrect parameter count in the call to native function 'AsBinary' query: geo_field /* anna : geo_field */
//      SELECT AsBinary() AS geo FROM WHERE vid = 12038 in /Library/WebServer/Documents/drupal6-local/sites/all/modules/geo/modules/geo_field/geo_field.module on line 83.
//       check $coordinates format!!!


//      echo 'HERE!';
//      dpr($coordinates);
//      print $coordinates;
//      $q = db_result(db_query("SELECT PointFromText('%s')", 'POINT(1 1)'));
////      $q = db_result(db_query("SELECT GeomFromText('%s')", 'POINT(31.5 42.2)'));
////      PointFromText(Point(a,b))
////      GeomFromWKB(Point(1,2)
//      $node->field_research_site_pt_coords[0]['geo'] = $q;
//      $wkt = geo_value($q, 'wkt', 'wkb');
//      dpr($wkt);
//      print_r($wkt);
//      parse 'Coordinates' here
      //    'field_research_site_pt_coords'   => 'Coordinates',
//      $node->field_geofield[0]['geo'] = 'POINT(30 30)';
//      $nofe->field_research_site_pt_coords[0]['geo'] = 'POINT(30 30)';
//      $node->field_research_site_pt_coords[0] = array('geo' => '');
//        $wkb = 'POINT(30 30)';
//        $g = GeomFromText($wkb);
//           $geo = geo_load();
//           $point = $geo->point(1, 2);
//
//        dpr($point);
//        dpr($wkb);
//        $geo = geo_load();
//        $geo = geo_load(array('table_name' => $field['geo']));
//          dpr(geo_field_names());
//          echo "UUUU";
//        dpr($geo);
//        dpm($geo);
//        $g = $geo->geomFromWKB($wkb);
//        dpr($g);
//        $wkt = $g->asText();
//
//$wkt = geo('wkt_from_point', 30, 30);
//echo 'UUU';
//geo_value($input, $output_format, $input_format = 'wkb')
//
//$wkt = geo_value($wkt, 'wkt', $gis_input);
//$wkb = geo_value($wkt, 'wkb', 'wkt');

//dpr(geo_list());
//dpm(geo_list());
//dpm(geo_field_names());
//dpr(geo_field_names());

//    [4] => GeoSQLMySQL Object
//        (
//            [gid:protected] => 4
//            [name:protected] => field_research_site_pt_coords
//            [title:protected] => Coordinates
//            [srid:protected] => 4326
//            [geo_type:protected] => 0
//            [table_name:protected] => content_type_research_site
//            [column_name:protected] => field_research_site_pt_coords_geo
//            [schema:protected] =>
//            [indexed:protected] => 1
//            [count:protected] =>
//        )
//
//)
//Array
//(
//    [0] => field_research_site_pt_coords
//)

//      $wkb_data = GeomFromText('POINT(30 30)');
//      dpr($wkb_data);
//      $data = geo_wkb_get_data('POINT(30 30)', 'wkt');
//      echo 'URRA!';
//      dpr($data);
//      print_r($wkb_data);

//      $node->field_research_site_pt_coords[0]['geo'] = $wkt;
//      $node->field_research_site_pt_coords[0]['geo'] = 'POINT(30 30)';
// $node->field_research_site_pt_coords_geo = 'POINT(30 30)';

//    dpr($node);


      node_save(&$node);
      $ref_nids[$content_type][] = $node->nid;
//      $site_nid = $node->nid;
//      $sql = "SELECT X(field_research_site_pt_coords_geo) as longitude,
//             Y(field_research_site_pt_coords_geo) as latitude,
//             AsText(field_research_site_pt_coords_geo) as geo_point
//             FROM {content_type_research_site}
//             WHERE vid=(SELECT max(vid)
//                        FROM {content_type_research_site}
//                        WHERE nid = '%d')";
//
//      $query = db_query($sql, $site_nid);
//      $geo_lon_lat_point =  db_fetch_array($query);
//      dpr($geo_lon_lat_point);
//      dpm($geo_lon_lat_point);

//      select GeomFromText(select GeomFromText('POINT(31.5 42.2)');


//      node_save(&$node);

//      dpr($node);
//      dpm($node);
//      print_r($node);

//      if (is_string($shp['data'])) {
//        $values['geo'] = db_result(db_query("SELECT GeomFromText('%s', %d)", $shp['data'], $srid));
//      } else {
//        $values['geo'] = db_result(db_query("SELECT GeomFromText('%s', %d)", $shp['data']['wkt'], $srid));
//      }


    }
//    dpr($node);
  }
  return $ref_nids;
}

function create_file_object($file) {
  $details = stat($file);
  $filesize = $details['size'];

  // Get the path to your Drupal site's files directory
  $dest = file_directory_path();

  // Copy the file to the Drupal files directory
  if(!file_copy($file, $dest)) {
      echo "Failed to move file: $file.\n";
      return;
  } else {
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
  $file_obj->new        = true;

  // Save file to files table
  drupal_write_record('files', $file_obj);
  // change file status to permanent
  file_set_status($file_obj, 1);

  // Attach the file object to your node
//  $node->files[$file_obj->fid] = $file_obj;
//  dpr($file_obj);
//  dpm($file_obj);

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
      $node = new StdClass();
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
//TODO: compare names and save only if it's the same
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
//    check only by Title!
    $nid = validate_uniqness($value, $fields_names, $content_type);
    if (!empty($nid)) {
      $ref_nids[$content_type][] = $nid;
    }
    elseif (!empty($value['Title'])) {
      $node = new StdClass();
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
//  dpm($ref_nids);
//  dpr($node);
//  dpm($node);
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

function display_report_page()
{
  $csv2drupal_submit_time = variable_get(csv2drupal_submit_time, time());
//  $ref_nids = variable_get('ref_nids', '');
//  dpm('$ref_nids = ');
//  dpm($ref_nids);
//  foreach ($ref_nids as $value) {
//    foreach ($value as $ref_nid) {
//      $sql = "SELECT nid FROM {node} WHERE nid = %d ORDER BY nid DESC";
//      $db_result = db_query($sql, $ref_nid);
//      $header       = array('Title', 'Nid');
//      $rows['data'] = array();
//      while ($res   = db_fetch_object($db_result)) {
//        $nid        = $res->nid;
//        $node       = node_load($nid);
//        $node_nid   = $node->nid;
//        $node_title = $node->title;
//        $node_type  = $node->type;
//        $node_link  = l($nid, 'node/'.$nid);
//        if ($node_type != $old_node_type) {
//          $rows[] = array(array('data' => 'Type = ' . $node_type, 'colspan' => 2, 'font size' => 4, 'align' => 'center'));
//          $old_node_type = $node_type;
//        }
//        $rows[] = array('Title = ' . $node_title, $node_link);
//      }
//    }
//  }
  $sql = "SELECT nid FROM {node} WHERE created >= %d ORDER BY nid DESC";
  $db_result = db_query($sql, $csv2drupal_submit_time);

  $header       = array('Title', 'Nid');
  $rows['data'] = array();
  while ($res   = db_fetch_object($db_result)) {
//    dpm('$res = ');
//    dpm($res);
    $nid        = $res->nid;
    $node       = node_load($nid);
    $node_nid   = $node->nid;
    $node_title = $node->title;
    $node_type  = $node->type;
    $node_link  = l($nid, 'node/'.$nid);
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

function _trim_text($str)
{
    $str = trim($str);
    $str = preg_replace('/ +/', ' ', $str);
    return $str;
}

function get_person_array($data) {
  $m = 0;
  $person_names = array();
  if (preg_match_all("/([^,]+)/", $data, $matches, PREG_SET_ORDER)) {
    foreach ($matches as $match) {
//        dpm('$matches = ');
//        dpm($matches);
      $m++;
      $person_names[] = _trim_text($match[1]);
    }
  }
  return array ($person_names, $m);
//  return $person_names;
}

function get_person_ref($dataset_value, $person_ref_nids) {
  $dataset_person_refs = array();
  $dataset_person_ref_fields = dataset_person_ref_fields();
  /*
   1) take all names, connect to role
   2) clean names up (get_person_array) > $names
   3) convert $names to array
   4) foreach $person_ref_nids load node
   5) take node->title
   6) if node->title in $names, save node-> field = nid of this name
   *
   * name <=> role
   * name <=> nid
   *
   * $person_ref_nids - nid - node - title
   * $dataset_value[role] - name
   *
   */
$i1 = $i2 = 0;
  // 1)-3)
  foreach ($dataset_person_ref_fields as $dataset_field_name => $dataset_role) {
    $a = get_person_array($dataset_value[$dataset_role]);
    $clean_names[$dataset_role] = $a[0];
    $b1 = $a[1];

//    $clean_names[$dataset_role] = get_person_array($dataset_value[$dataset_role]);
    $i1 = $i1 + $b1;
    $i1++;
  }

  // 4)-5)
  foreach ($person_ref_nids as $person_ref_nid) {
    $node = node_load($person_ref_nid);
    $title = $node->title;
    $titles_n_nids[$person_ref_nid] = _trim_text($title);
    $i1++;
  }

  // 6)
  foreach ($clean_names as $role => $value) {
    if (!empty($value)) {
      $dataset_field_name = array_search($role, $dataset_person_ref_fields);
      foreach ($value as $clean_name) {
        $nid = array_search($clean_name, $titles_n_nids);
        $dataset_person_refs[$dataset_field_name][] = $nid;
    $i1++;
      }
    }
  }

//  dpm('1) $i1 = ' . $i1);
////        dpm('1) $dataset_person_refs = ');
////        dpm($dataset_person_refs);
//
//$dataset_person_refs = array();
//  foreach ($dataset_person_ref_fields as $dataset_field_name => $dataset_role) {
//    $b = get_person_array($dataset_value[$dataset_role]);
//    $name = $b[0];
//    $b2 = $b[1];
//    $i2 = $i2 + $b2;
//
////    $name = get_person_array($dataset_value[$dataset_role]);
//    is_array($name) ? $names = $name : $names = (array) $name;
//    if (!empty ($person_ref_nids)) {
//      foreach ($person_ref_nids as $person_ref_nid) {
//        $node = node_load($person_ref_nid);
////        dpm('$node ');
//
//        $title = $node->title;
//        if (in_array(trim($title), $names)) {
//          $dataset_person_refs[$dataset_field_name][] = $person_ref_nid;
//    $i2++;
//        }
//      }
//    }
//  }
//    dpm('2) $i2 = ' . $i2);

//        dpm('2) $dataset_person_refs = ');
//        dpm($dataset_person_refs);
  return $dataset_person_refs;
}

/*
 function get_person_ref($dataset_value, $person_ref_nids) {
  $dataset_person_refs = array();
  $dataset_person_ref_fields = dataset_person_ref_fields();
  foreach ($dataset_person_ref_fields as $dataset_field_name => $dataset_role) {
//    $name = $dataset_value[$dataset_role];
    $name = get_person_array($dataset_value[$dataset_role]);
    is_array($name) ? $names = $name : $names = (array) $name;
    if (!empty ($person_ref_nids)) {
      foreach ($person_ref_nids as $person_ref_nid) {
        dpm('$person_ref_nid = ');
        dpm($person_ref_nid);
        $node = node_load($person_ref_nid);
        dpm('$node ');

        $title = $node->title;
        if (in_array(trim($title), $names)) {
          $dataset_person_refs[$dataset_field_name][] = $person_ref_nid;
        }
      }
    }
  }
        dpm('$dataset_person_refs = ');
        dpm($dataset_person_refs);
  return $dataset_person_refs;
}

 */

//function get_person_ref($dataset_value, $person_ref_nids) {
//  $dataset_person_refs = array();
//  $dataset_person_ref_fields = dataset_person_ref_fields();
//  $person_titles_n_nids = array();
//  if (!empty ($person_ref_nids)) {
//    foreach ($person_ref_nids as $person_ref_nid) {
//      $node = node_load($person_ref_nid);
//      $title = $node->title;
//      $person_titles_n_nids[] = array($person_ref_nid => trim($title));
//    }
//
//    foreach ($dataset_person_ref_fields as $dataset_field_name => $dataset_role) {
//      $name = get_person_array($dataset_value[$dataset_role]);
//      if (!empty($name)) {
//        is_array($name) ? $names = $name : $names = (array) $name;
////        TODO: print $person_titles_n_nids, $names and find intersect
//
//        dpm('$person_titles_n_nids = ');
//        dpm($person_titles_n_nids);
//        dpm('$names = ');
//        dpm($names);
//        forea
//        $inter = array_intersect($person_titles_n_nids, $names);
//        if (!empty($inter)) {
//          dpm('array_intersect($person_titles_n_nids, $names = ');
//          dpm($inter);
//          $dataset_person_refs[$dataset_field_name][] = $person_ref_nid;
//        }
//      }
//    }
//  }

//  foreach ($dataset_person_ref_fields as $dataset_field_name => $dataset_role) {
//    $name = $dataset_value[$dataset_role];
//    is_array($name) ? $names = $name : $names = (array) $name;
//    if (!empty ($person_ref_nids)) {
//      foreach ($person_ref_nids as $person_ref_nid) {
//        dpm('$person_ref_nid = ');
//        dpm($person_ref_nid);
//        $node = node_load($person_ref_nid);
//        $title = $node->title;
//        if (in_array(trim($title), $names)) {
//          $dataset_person_refs[$dataset_field_name][] = $person_ref_nid;
//        }
//      }
//    }
//  }
//  return $dataset_person_refs;
//}

/*

//function create_values($content) {
//  //TODO: why person in common method?
//    dpm($content);
//
//  $dataset_person_ref_fields = dataset_person_ref_fields();
//  $values = array();
//  if ($content) {
//    foreach ($content as $key => $value) {
//      foreach ($value as $number => $data) {
//        if (in_array($key, $dataset_person_ref_fields) AND (preg_match("/,/", $data))) {
//          $data = get_person_array($data);
//        }
//        $values[$number][$key] = $data;
//      }
//    }
//  }
//  return $values;
//}


 */


function add_person_refs(&$node, $dataset_person_refs) {
//  $dataset_person_refs wrong
  foreach ($dataset_person_refs as $field_name => $value1) {
//    dpm('$field_name = ');
//    dpm($field_name);
    $refs = array();
    foreach ($value1 as $ref) {
      $refs[]['nid'] = $ref;
    }
//      dpm('$refs');
//      dpm($refs);
    $node->$field_name = $refs;
  }
  return $node;
}
