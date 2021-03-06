/****************************************************************************/
/*                                                                          */
/*                         Copyright or (C) or Copr.                        */
/*       Commissariat a l'Energie Atomique et aux Energies Alternatives     */
/*                                                                          */
/* Version : 1.2                                                            */
/* Date    : Tue Jul 22 13:28:10 CEST 2014                                  */
/* Ref ID  : IDDN.FR.001.160040.000.S.P.2015.000.10800                      */
/* Author  : Julien Adam <julien.adam@cea.fr>                               */
/*           Marc Perache <marc.perache@cea.fr>                             */
/*                                                                          */
/* This file is part of JCHRONOSS software.                                 */
/*                                                                          */
/* This software is governed by the CeCILL-C license under French law and   */
/* abiding by the rules of distribution of free software.  You can  use,    */
/* modify and/or redistribute the software under the terms of the CeCILL-C  */
/* license as circulated by CEA, CNRS and INRIA at the following URL        */
/* "http://www.cecill.info".                                                */
/*                                                                          */
/* As a counterpart to the access to the source code and  rights to copy,   */
/* modify and redistribute granted by the license, users are provided only  */
/* with a limited warranty  and the software's author,  the holder of the   */
/* economic rights,  and the successive licensors  have only  limited       */
/* liability.                                                               */
/*                                                                          */
/* In this respect, the user's attention is drawn to the risks associated   */
/* with loading,  using,  modifying and/or developing or reproducing the    */
/* software by the user in light of its specific status of free software,   */
/* that may mean  that it is complicated to manipulate,  and  that  also    */
/* therefore means  that it is reserved for developers  and  experienced    */
/* professionals having in-depth computer knowledge. Users are therefore    */
/* encouraged to load and test the software's suitability as regards their  */
/* requirements in conditions enabling the security of their systems and/or */
/* data to be ensured and,  more generally, to use and operate it in the    */
/* same conditions as regards security.                                     */
/*                                                                          */
/* The fact that you are presently reading this means that you have had     */
/* knowledge of the CeCILL-C license and that you accept its terms.         */
/*                                                                          */
/****************************************************************************/

#ifndef OUTPUTFORMATYAML_H
#define OUTPUTFORMATYAML_H

#include "OutputFormat.h"

/**
 * OutputFormat implementation for YAML format.
 */
class OutputFormatYAML : public OutputFormat
{
	private:
		std::ostringstream flux; ///< container handling YAML-formatted data.
	public:
	/**
	 * default constructor
	 * \param[in] config the config
	 */
	explicit OutputFormatYAML(Configuration *config);

	virtual std::string getName();
	virtual std::string getExt();
	virtual void appendError(std::string group, std::string name, std::string command, std::string data, double time);
	virtual void appendFailure(std::string group, std::string name, std::string command, std::string data, double time);
	virtual void appendDisabled(std::string group, std::string name, std::string command, std::string data, double time);
	virtual void appendSkipped(std::string group, std::string name, std::string command, std::string data, double time);
	virtual void appendSuccess(std::string group, std::string name, std::string command, std::string data, double time);
	virtual void appendHeader(size_t err, size_t fail, size_t skip, size_t succ, double time);
	virtual void appendFooter();

	virtual bool isEmpty();
	virtual std::string stringify();
	virtual void clear();
	~OutputFormatYAML();
};


#endif /* OUTPUTMODULE_H */
