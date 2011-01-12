<?php

function dataset_fields() {
  $dataset_fields = array(
    'field_beg_end_date'              => 'Date Range',
    'field_dataset_abstract'          => 'Abstract',
    'field_dataset_add_info' 				  => 'Additional Information',
    'field_dataset_id' 						    => 'Dataset ID',
    'field_dataset_maintenance'       => 'Maintenance',
    'field_dataset_publication_date'  => 'Publication Date',
    'field_dataset_purpose' 					=> 'Purpose',
    'field_dataset_related_links' 		=> 'Related Links',
    'field_dataset_short_name'    		=> 'Short Name',
    'field_instrumentation'       		=> 'Instrumentation',
    'field_methods' 									=> 'Methods',
    'field_quality' 									=> 'Quality Assurance',
  );
  return $dataset_fields;
}

function datafile_fields() {
  $datafile_fields = array (
    'field_datafile_description'  => 'Description',
    'field_data_file'             => 'Data File',
    'field_num_header_line'       => 'Number of Header Lines',
    'field_num_footer_lines'      => 'Number of Footer Lines',
    'field_orientation'           => 'Orientation',
    'field_quote_character'       => 'Quote Character',
    'field_delimiter'             => 'Field Delimiter',
    'field_record_delimiter'      => 'Record Delimiter',
    'field_beg_end_date'          => 'Date Range',
    'field_methods'               => 'Methods',
    'field_instrumentation'       => 'Instrumentation',
    'field_quality'               => 'Quality Assurance',
  );
  return $datafile_fields;
}

